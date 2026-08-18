#pragma once
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QProcess>
#include <QStandardPaths>

// ─────────────────────────────────────────────────────────────────────────────
// OsDetect — runtime OS identification
//
// Supports: Windows, macOS, Arch Linux, Debian/Ubuntu, Fedora/RHEL
// Detection is cached after first call; all methods are static.
// ─────────────────────────────────────────────────────────────────────────────

class OsDetect
{
public:
    enum class OS {
        Windows,
        macOS,
        Arch,       // pacman-based (Arch, Manjaro, EndeavourOS…)
        Debian,     // apt-based  (Debian, Ubuntu, Mint, Pop!_OS…)
        Fedora,     // dnf-based  (Fedora, RHEL, AlmaLinux, Rocky…)
        Void,       // xbps-based (Void Linux)
        Unknown
    };

    // ── Primary accessor ──────────────────────────────────────────────────────
    static OS current()
    {
        static OS cached = detect();
        return cached;
    }

    // ── Human-readable name ───────────────────────────────────────────────────
    static QString name()
    {
        switch (current()) {
            case OS::Windows: return windowsEdition();
            case OS::macOS:   return macOSName();
            case OS::Arch:    return archDistroName();
            case OS::Debian:  return debianDistroName();
            case OS::Fedora:  return fedoraDistroName();
            case OS::Void:    return voidLinuxDistroName();
            default:          return "Unknown OS";
        }
    }

    // ── Short tag (used for icon / badge) ─────────────────────────────────────
    static QString tag()
    {
        switch (current()) {
            case OS::Windows: return "Windows";
            case OS::macOS:   return "macOS";
            case OS::Arch:    return "Arch";
            case OS::Debian:  return "Debian";
            case OS::Fedora:  return "Fedora";
            case OS::Void:    return "Void";
            default:          return "?";
        }
    }

    // ── Package manager command prefix ────────────────────────────────────────
    // Returns the install invocation prefix (caller appends package name).
    static QString pkgInstallPrefix()
    {
        switch (current()) {
            case OS::Windows: return "winget install --silent --accept-package-agreements --accept-source-agreements";
            case OS::macOS:   return "brew install";
            case OS::Arch:    return "sudo pacman -S --noconfirm";
            case OS::Debian:  return "sudo apt-get install -y";
            case OS::Fedora:  return "sudo dnf install -y";
            case OS::Void:    return "sudo xbps-install -Sy";
            default:          return "";
        }
    }

    // ── Elevation note ────────────────────────────────────────────────────────
    // True when installs need sudo/UAC (always true on Linux/macOS package mgrs)
    static bool requiresElevation()
    {
        switch (current()) {
            case OS::Windows: return false; // winget handles UAC itself
            case OS::macOS:   return false; // brew runs as user
            default:          return true;  // Linux distros use sudo
        }
    }

    // ── Availability checks ───────────────────────────────────────────────────
    static bool isWindows() { return current() == OS::Windows; }
    static bool isMacOS()   { return current() == OS::macOS;   }
    static bool isArch()    { return current() == OS::Arch;    }
    static bool isDebian()  { return current() == OS::Debian;  }
    static bool isFedora()  { return current() == OS::Fedora;  }
    static bool isVoid()    { return current() == OS::Void;    }
    static bool isLinux()   { return isArch() || isDebian() || isFedora() || isVoid(); }

private:
    // ── Core detection logic ──────────────────────────────────────────────────
    static OS detect()
    {
#if defined(Q_OS_WIN)
        return OS::Windows;

#elif defined(Q_OS_MACOS)
        return OS::macOS;

#elif defined(Q_OS_LINUX)
        // Read /etc/os-release for distro identity
        QString osRelease = readFile("/etc/os-release");
        if (osRelease.isEmpty())
            osRelease = readFile("/usr/lib/os-release");

        const QString lower = osRelease.toLower();

        // Arch and derivatives: ID=arch  /  ID_LIKE=arch
        if (lower.contains("id=arch") || lower.contains("id_like=arch")
            || lower.contains("id=manjaro") || lower.contains("id=endeavouros")
            || lower.contains("id=garuda") || lower.contains("id=cachyos"))
            return OS::Arch;

        // Fedora / RHEL family
        if (lower.contains("id=fedora") || lower.contains("id_like=fedora")
            || lower.contains("id=rhel")  || lower.contains("id=centos")
            || lower.contains("id=almalinux") || lower.contains("id=rocky"))
            return OS::Fedora;

        // Void Linux (xbps-based, NOT debian)
        if (lower.contains("id=void") || lower.contains("id_like=void"))
            return OS::Void;

        // Debian / Ubuntu family (check after Fedora to avoid false ID_LIKE hits)
        if (lower.contains("id=debian") || lower.contains("id=ubuntu")
            || lower.contains("id_like=debian") || lower.contains("id_like=ubuntu")
            || lower.contains("id=linuxmint") || lower.contains("id=pop")
            || lower.contains("id=elementary") || lower.contains("id=kali"))
            return OS::Debian;

        // Fallback: check for package manager binaries
        if (QFile::exists("/usr/bin/pacman") || QFile::exists("/bin/pacman"))
            return OS::Arch;
        if (QFile::exists("/usr/bin/dnf") || QFile::exists("/bin/dnf"))
            return OS::Fedora;
        if (QFile::exists("/usr/bin/apt-get") || QFile::exists("/bin/apt-get"))
            return OS::Debian;
        if (QFile::exists("/usr/bin/xbps-install") || QFile::exists("/bin/xbps-install"))
            return OS::Void;

        return OS::Unknown;
#else
        return OS::Unknown;
#endif
    }

    // ── Helper: read entire text file ─────────────────────────────────────────
    static QString readFile(const QString &path)
    {
        QFile f(path);
        if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
            return {};
        return QTextStream(&f).readAll();
    }

    // ── Helper: read a specific field from /etc/os-release ───────────────────
    static QString osReleaseField(const QString &key)
    {
        const QString content = readFile("/etc/os-release");
        for (const QString &line : content.split('\n')) {
            if (line.startsWith(key + "=", Qt::CaseInsensitive)) {
                QString val = line.mid(key.length() + 1).trimmed();
                // Strip surrounding quotes
                if (val.startsWith('"') && val.endsWith('"'))
                    val = val.mid(1, val.length() - 2);
                return val;
            }
        }
        return {};
    }

    // ── Pretty-name helpers ───────────────────────────────────────────────────
    static QString windowsEdition()
    {
#if defined(Q_OS_WIN)
        // Query registry for Windows edition string
        QProcess p;
        p.start("cmd", { "/c", "ver" });
        p.waitForFinished(3000);
        QString out = QString::fromLocal8Bit(p.readAllStandardOutput()).trimmed();
        if (!out.isEmpty()) return out;
#endif
        return "Windows";
    }

    static QString macOSName()
    {
#if defined(Q_OS_MACOS)
        QProcess p;
        p.start("sw_vers", { "-productVersion" });
        p.waitForFinished(3000);
        QString ver = QString::fromLocal8Bit(p.readAllStandardOutput()).trimmed();
        if (!ver.isEmpty()) return "macOS " + ver;
#endif
        return "macOS";
    }

    static QString archDistroName()
    {
        QString name = osReleaseField("PRETTY_NAME");
        return name.isEmpty() ? "Arch Linux" : name;
    }

    static QString debianDistroName()
    {
        QString name = osReleaseField("PRETTY_NAME");
        return name.isEmpty() ? "Debian/Ubuntu" : name;
    }

    static QString fedoraDistroName()
    {
        QString name = osReleaseField("PRETTY_NAME");
        return name.isEmpty() ? "Fedora" : name;
    }

    static QString voidLinuxDistroName()
    {
        QString name = osReleaseField("PRETTY_NAME");
        return name.isEmpty() ? "Void Linux" : name;
    }
};
