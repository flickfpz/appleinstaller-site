#include "InstallWorker.h"
#include "OsDetect.h"

#include <QProcess>
#include <QThread>
#include <QProcessEnvironment>

InstallWorker::InstallWorker(const QVector<AppData> &apps, QObject *parent)
    : QObject(parent), m_apps(apps)
{}

// ── run ───────────────────────────────────────────────────────────────────────

void InstallWorker::run()
{
    const int total = m_apps.size();
    emit started(total);

    int succeeded = 0, failed = 0, skipped = 0;

    for (int i = 0; i < total; ++i) {
        const AppData &app = m_apps[i];
        emit progress(i, total, app.name);

        InstallResult result = runCommand(app);
        emit appFinished(result);

        if      (result.skipped) ++skipped;
        else if (result.success) ++succeeded;
        else                     ++failed;

        QThread::msleep(150);
    }

    emit progress(total, total, QString());
    emit finished(succeeded, failed, skipped);
}

// ── runCommand ────────────────────────────────────────────────────────────────

InstallResult InstallWorker::runCommand(const AppData &app)
{
    InstallResult result;
    result.id   = app.id;
    result.name = app.name;

    const OsCmd cmd = app.cmdForCurrentOs();

    // ── Not available on this OS ──────────────────────────────────────────────
    if (!cmd.available || cmd.program.isEmpty()) {
        result.skipped = true;
        result.success = false;
        result.output  = QStringLiteral("%1 is not available on %2.")
                            .arg(app.name, OsDetect::name());
        return result;
    }

    // ── Build environment ─────────────────────────────────────────────────────
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();

    // Homebrew on Apple Silicon lives in /opt/homebrew/bin
    // Homebrew on Intel lives in /usr/local/bin
    // Add both to PATH so brew is always found regardless of shell init
#if defined(Q_OS_MACOS)
    QString path = env.value("PATH");
    if (!path.contains("/opt/homebrew/bin"))
        path.prepend("/opt/homebrew/bin:");
    if (!path.contains("/usr/local/bin"))
        path.prepend("/usr/local/bin:");
    env.insert("PATH", path);
    // Prevent brew from opening a browser for analytics prompts
    env.insert("HOMEBREW_NO_ANALYTICS", "1");
    env.insert("HOMEBREW_NO_AUTO_UPDATE", "1");
    env.insert("HOMEBREW_NO_ENV_HINTS", "1");
#endif

#if defined(Q_OS_WIN)
    // winget needs these to avoid interactive prompts
    env.insert("WINGET_DISABLE_TELEMETRY", "1");
#endif

    // ── Launch process ────────────────────────────────────────────────────────
    QProcess proc;
    proc.setProcessEnvironment(env);
    proc.setProcessChannelMode(QProcess::MergedChannels);

    // On Linux/macOS, sudo commands need a DISPLAY-less or -S (stdin) invocation.
    // We pass SUDO_ASKPASS-style: since the GUI app owns the session, sudo should
    // have already been authenticated (or the user ran with sudo), but we add
    // "-n" (non-interactive) to fail fast rather than hang waiting for a password.
    QString program     = cmd.program;
    QStringList args    = cmd.args;

    if (program == "sudo" && OsDetect::isLinux()) {
        // Insert -n after "sudo" so it never prompts — if it would prompt, fail.
        args.prepend("-n");
    }

    proc.start(program, args);

    if (!proc.waitForStarted(8000)) {
        result.success = false;
        result.output  = QStringLiteral(
            "Could not start '%1'. Is it installed and in PATH?\n"
            "OS: %2\nArgs: %3")
            .arg(program, OsDetect::name(), args.join(' '));
        return result;
    }

    // Per-app timeout: 10 minutes (large packages like Xcode tools can be slow)
    const bool finished = proc.waitForFinished(600'000);

    result.output = QString::fromLocal8Bit(proc.readAll()).trimmed();

    if (!finished) {
        proc.kill();
        result.success = false;
        result.output.prepend("[TIMEOUT — process killed]\n");
        return result;
    }

    // winget uses 0 for success and also for "already installed"
    // apt/dnf/pacman use 0 for success
    const int code = proc.exitCode();
    result.success = (proc.exitStatus() == QProcess::NormalExit) &&
                     (code == 0 || isAlreadyInstalledCode(program, code));

    if (!result.success && result.output.isEmpty())
        result.output = QStringLiteral("Exit code %1").arg(code);

    return result;
}

// ── isAlreadyInstalledCode ────────────────────────────────────────────────────
// Some package managers return non-zero when a package is already installed,
// which we treat as success.

bool InstallWorker::isAlreadyInstalledCode(const QString &program, int code)
{
    // winget: 0x8A150011 (2317254673) = APPINSTALLER_CLI_ERROR_PACKAGE_ALREADY_INSTALLED
    //         cast to int on 32-bit = -1977712623; on 64-bit signed int varies
    //         The hex value is what winget docs say; check both.
    if (program == "winget") {
        constexpr unsigned int wingetAlreadyInstalled = 0x8A150011u;
        return (unsigned int)code == wingetAlreadyInstalled;
    }
    // brew: exit 0 always; no special code needed
    // apt: exit 0 always for already-installed
    return false;
}
