#include "UpdateChecker.h"
#include "Theme.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkRequest>
#include <QUrl>
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QTemporaryFile>
#include <QTextStream>
#include <QDirIterator>
#include <QStandardPaths>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QProgressBar>
#include <QPainter>
#include <QPainterPath>
#include <QGraphicsDropShadowEffect>

static const QString VERSION_URL =
    QStringLiteral("https://flickfpz.github.io/rigset/version.json");

UpdateChecker::UpdateChecker(QObject *parent)
    : QObject(parent)
    , m_currentVersion(QApplication::applicationVersion())
{
    connect(&m_nam, &QNetworkAccessManager::finished,
            this, &UpdateChecker::onVersionReplyFinished);
}

void UpdateChecker::check()
{
    QNetworkRequest req{QUrl(VERSION_URL)};
    req.setTransferTimeout(5000);
    m_nam.get(req);
}

void UpdateChecker::onVersionReplyFinished(QNetworkReply *reply)
{
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError)
        return;

    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    if (!doc.isObject())
        return;

    QJsonObject root = doc.object();
    QString remoteVer = root.value("version").toString();
    if (remoteVer.isEmpty())
        return;

    if (!isNewer(remoteVer, m_currentVersion))
        return;

    QString url;
#if defined(Q_OS_WIN)
    url = root.value("download").toObject().value("windows").toString();
#elif defined(Q_OS_MACOS)
    url = root.value("download").toObject().value("macos").toString();
#else
    url = root.value("download").toObject().value("linux").toString();
#endif

    if (url.isEmpty())
        return;

    emit updateRequired(remoteVer, url);
}

void UpdateChecker::startDownload(const QString &url)
{
    if (m_downloadReply) {
        m_downloadReply->abort();
        m_downloadReply->deleteLater();
        m_downloadReply = nullptr;
    }

    QNetworkRequest req{QUrl(url)};
    req.setTransferTimeout(120000);

    // Disconnect the version-check handler so it doesn't fire for download replies
    disconnect(&m_nam, &QNetworkAccessManager::finished,
               this, &UpdateChecker::onVersionReplyFinished);

    m_downloadReply = m_nam.get(req);

    connect(m_downloadReply, &QNetworkReply::downloadProgress,
            this, &UpdateChecker::onDownloadProgress);
    connect(m_downloadReply, &QNetworkReply::finished,
            this, &UpdateChecker::onDownloadFinished);
    connect(m_downloadReply, &QNetworkReply::errorOccurred,
            this, &UpdateChecker::onDownloadError);
}

void UpdateChecker::cancelDownload()
{
    if (m_downloadReply) {
        m_downloadReply->abort();
        m_downloadReply->deleteLater();
        m_downloadReply = nullptr;
    }
}

void UpdateChecker::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    emit downloadProgress(bytesReceived, bytesTotal);
}

void UpdateChecker::onDownloadFinished()
{
    if (!m_downloadReply)
        return;

    if (m_downloadReply->error() != QNetworkReply::NoError) {
        m_downloadReply->deleteLater();
        m_downloadReply = nullptr;
        return;
    }

    QByteArray data = m_downloadReply->readAll();
    m_downloadReply->deleteLater();
    m_downloadReply = nullptr;

    emit statusChanged(QStringLiteral("Saving update..."));

    QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    QString filePath;

#if defined(Q_OS_WIN)
    filePath = tempDir + QStringLiteral("/rigset-update.zip");
#elif defined(Q_OS_MACOS)
    filePath = tempDir + QStringLiteral("/rigset-update.tar.gz");
#else
    filePath = tempDir + QStringLiteral("/rigset-update.tar.gz");
#endif

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        emit downloadFailed(QStringLiteral("Cannot write to temp directory"));
        return;
    }
    file.write(data);
    file.close();

    emit downloadFinished(filePath);
}

void UpdateChecker::onDownloadError(QNetworkReply::NetworkError error)
{
    if (error == QNetworkReply::OperationCanceledError)
        return;

    emit downloadFailed(QStringLiteral("Network error: %1").arg(
        m_downloadReply ? m_downloadReply->errorString() : QStringLiteral("unknown")));
    if (m_downloadReply) {
        m_downloadReply->deleteLater();
        m_downloadReply = nullptr;
    }
}

void UpdateChecker::applyUpdate(const QString &downloadedFile)
{
    emit statusChanged(QStringLiteral("Extracting update..."));

    QString appDir = QCoreApplication::applicationDirPath();

#if defined(Q_OS_WIN)
    // Windows: extract zip with PowerShell, then write a batch script to replace
    QString extractDir = appDir + QStringLiteral("/_update_tmp");
    QDir().mkpath(extractDir);

    QProcess extract;
    extract.start(QStringLiteral("powershell"), {
        QStringLiteral("-NoProfile"), QStringLiteral("-Command"),
        QStringLiteral("Expand-Archive -Path '%1' -DestinationPath '%2' -Force")
            .arg(downloadedFile, extractDir)
    });
    extract.waitForFinished(60000);

    if (extract.exitCode() != 0) {
        emit downloadFailed(QStringLiteral("Extraction failed: %1").arg(extract.readAllStandardError()));
        QDir(extractDir).removeRecursively();
        return;
    }

    // Find the new Rigset.exe in extracted content
    QString newExe;
    QDirIterator it(extractDir, {"Rigset.exe", "rigset.exe"}, QDir::Files, QDirIterator::Subdirectories);
    if (it.hasNext()) {
        newExe = it.next();
    }
    if (newExe.isEmpty()) {
        emit downloadFailed(QStringLiteral("Rigset.exe not found in update package"));
        QDir(extractDir).removeRecursively();
        return;
    }

    emit statusChanged(QStringLiteral("Installing update..."));

    // Write a batch script that waits, replaces, and relaunches
    QString batPath = QDir::temp().filePath(QStringLiteral("rigset-update.bat"));
    QFile bat(batPath);
    if (bat.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream ts(&bat);
        ts << "@echo off\n";
        ts << "timeout /t 2 /nobreak >nul\n";
        ts << "taskkill /f /im Rigset.exe >nul 2>&1\n";
        ts << "timeout /t 1 /nobreak >nul\n";
        ts << "copy /y \"" << QDir::toNativeSeparators(newExe) << "\" \"" << QDir::toNativeSeparators(QCoreApplication::applicationFilePath()) << "\" >nul\n";
        ts << "rmdir /s /q \"" << QDir::toNativeSeparators(extractDir) << "\" >nul 2>&1\n";
        ts << "del \"%~f0\" >nul 2>&1\n";
        ts << "start \"\" \"" << QDir::toNativeSeparators(QCoreApplication::applicationFilePath()) << "\"\n";
        bat.close();
    }

    QProcess::startDetached(QStringLiteral("cmd"), { QStringLiteral("/c"), batPath });
    QApplication::quit();

#else
    // Linux/macOS: extract tar.gz
    QProcess extract;
    extract.start(QStringLiteral("tar"), {
        QStringLiteral("-xzf"), downloadedFile,
        QStringLiteral("-C"), appDir
    });
    extract.waitForFinished(60000);

    if (extract.exitCode() != 0) {
        emit downloadFailed(QStringLiteral("Extraction failed: %1").arg(extract.readAllStandardError()));
        return;
    }

    // On macOS the tar may contain a .app bundle — find and move it
#if defined(Q_OS_MACOS)
    QString appName = QCoreApplication::applicationName();
    QString appPath = appDir + QStringLiteral("/") + appName + QStringLiteral(".app");
    if (!QFileInfo::exists(appPath)) {
        // Look for any .app in extracted content
        QDirIterator it(appDir, {"*.app"}, QDir::Dirs, QDirIterator::Subdirectories);
        if (it.hasNext()) {
            QString found = it.next();
            // Move to /Applications or alongside
            appPath = found;
        }
    }
    emit statusChanged(QStringLiteral("Restarting..."));
    QProcess::startDetached(QStringLiteral("/bin/bash"), {
        QStringLiteral("-c"),
        QStringLiteral("sleep 1 && open \"%1\"").arg(appPath)
    });
#else
    // Linux: make sure binary is executable
    QString binaryPath = appDir + QStringLiteral("/rigset");
    QFile::setPermissions(binaryPath,
        QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner |
        QFileDevice::ReadGroup | QFileDevice::ExeGroup |
        QFileDevice::ReadOther | QFileDevice::ExeOther);

    emit statusChanged(QStringLiteral("Restarting..."));
    QProcess::startDetached(binaryPath, {}, appDir);
#endif

    QApplication::quit();
#endif
}

// ═════════════════════════════════════════════════════════════════════════════
// UpdateOverlay — full-screen blocking widget with live download progress
// ═════════════════════════════════════════════════════════════════════════════

class UpdateOverlay : public QWidget
{
    Q_OBJECT
public:
    UpdateOverlay(QWidget *parent, const QString &latestVersion,
                  const QString &downloadUrl, UpdateChecker *checker)
        : QWidget(parent)
        , m_checker(checker)
        , m_downloadUrl(downloadUrl)
    {
        setAttribute(Qt::WA_DeleteOnClose);
        setWindowFlags(Qt::Widget);
        setAttribute(Qt::WA_TranslucentBackground);
        raise();

        auto *card = new QFrame(this);
        card->setObjectName("updateCard");
        card->setFixedSize(460, 340);

        auto *cardShadow = new QGraphicsDropShadowEffect(card);
        cardShadow->setBlurRadius(40);
        cardShadow->setOffset(0, 8);
        cardShadow->setColor(QColor(0, 0, 0, 100));
        card->setGraphicsEffect(cardShadow);

        auto *cardLayout = new QVBoxLayout(card);
        cardLayout->setContentsMargins(36, 32, 36, 28);
        cardLayout->setSpacing(0);

        auto *iconLabel = new QLabel(QStringLiteral("\u2B06"));
        iconLabel->setAlignment(Qt::AlignCenter);
        QFont iconFont = iconLabel->font();
        iconFont.setPixelSize(40);
        iconLabel->setFont(iconFont);
        cardLayout->addWidget(iconLabel);
        cardLayout->addSpacing(12);

        m_titleLabel = new QLabel(QStringLiteral("Update Required"));
        m_titleLabel->setAlignment(Qt::AlignCenter);
        m_titleLabel->setFont(ThemeManager::fontTitle());
        m_titleLabel->setObjectName("titleLabel");
        cardLayout->addWidget(m_titleLabel);
        cardLayout->addSpacing(10);

        m_versionLabel = new QLabel(
            QStringLiteral("You're running v%1 \u2014 v%2 is available")
                .arg(QApplication::applicationVersion(), latestVersion));
        m_versionLabel->setAlignment(Qt::AlignCenter);
        m_versionLabel->setWordWrap(true);
        m_versionLabel->setFont(ThemeManager::fontBody());
        m_versionLabel->setObjectName("versionLabel");
        cardLayout->addWidget(m_versionLabel);
        cardLayout->addSpacing(6);

        m_statusLabel = new QLabel(QStringLiteral("Ready to download"));
        m_statusLabel->setAlignment(Qt::AlignCenter);
        m_statusLabel->setFont(ThemeManager::fontBody());
        m_statusLabel->setObjectName("statusLabel");
        cardLayout->addWidget(m_statusLabel);
        cardLayout->addSpacing(10);

        m_progressBar = new QProgressBar;
        m_progressBar->setRange(0, 100);
        m_progressBar->setValue(0);
        m_progressBar->setTextVisible(true);
        m_progressBar->setFormat("%p%");
        m_progressBar->setFixedHeight(20);
        m_progressBar->setObjectName("progressBar");
        cardLayout->addWidget(m_progressBar);
        cardLayout->addStretch();

        auto *btnRow = new QHBoxLayout;
        btnRow->setSpacing(12);

        m_cancelBtn = new QPushButton(QStringLiteral("Cancel"));
        m_cancelBtn->setObjectName("quitBtn");
        m_cancelBtn->setCursor(Qt::PointingHandCursor);
        m_cancelBtn->setFixedHeight(38);
        m_cancelBtn->setMinimumWidth(110);
        m_cancelBtn->setFont(ThemeManager::fontHeading());

        m_downloadBtn = new QPushButton(QStringLiteral("Download Update"));
        m_downloadBtn->setObjectName("updateBtn");
        m_downloadBtn->setCursor(Qt::PointingHandCursor);
        m_downloadBtn->setFixedHeight(38);
        m_downloadBtn->setMinimumWidth(150);
        m_downloadBtn->setFont(ThemeManager::fontHeading());
        m_downloadBtn->setDefault(true);

        btnRow->addWidget(m_cancelBtn);
        btnRow->addWidget(m_downloadBtn);
        cardLayout->addLayout(btnRow);

        connect(m_downloadBtn, &QPushButton::clicked, this, [this]() {
            m_downloadBtn->setEnabled(false);
            m_downloadBtn->setText(QStringLiteral("Downloading..."));
            m_statusLabel->setText(QStringLiteral("Starting download..."));
            m_checker->startDownload(m_downloadUrl);
        });
        connect(m_cancelBtn, &QPushButton::clicked, this, [this]() {
            m_checker->cancelDownload();
            close();
        });

        connect(m_checker, &UpdateChecker::downloadProgress, this,
            [this](qint64 received, qint64 total) {
            if (total > 0) {
                int pct = static_cast<int>(received * 100 / total);
                m_progressBar->setValue(pct);
                double mbR = received / (1024.0 * 1024.0);
                double mbT = total / (1024.0 * 1024.0);
                m_statusLabel->setText(
                    QStringLiteral("Downloading... %1 MB / %2 MB")
                        .arg(mbR, 0, 'f', 1).arg(mbT, 0, 'f', 1));
            } else {
                m_statusLabel->setText(
                    QStringLiteral("Downloading... %1 MB")
                        .arg(received / (1024.0 * 1024.0), 0, 'f', 1));
            }
        });

        connect(m_checker, &UpdateChecker::statusChanged, this,
            [this](const QString &status) {
            m_statusLabel->setText(status);
        });

        connect(m_checker, &UpdateChecker::downloadFinished, this,
            [this](const QString &path) {
            m_progressBar->setValue(100);
            m_statusLabel->setText(QStringLiteral("Applying update..."));
            m_downloadBtn->setText(QStringLiteral("Installing..."));
            m_cancelBtn->setEnabled(false);
            m_checker->applyUpdate(path);
        });

        connect(m_checker, &UpdateChecker::downloadFailed, this,
            [this](const QString &err) {
            m_statusLabel->setText(QStringLiteral("Failed: ") + err);
            m_downloadBtn->setEnabled(true);
            m_downloadBtn->setText(QStringLiteral("Retry"));
            m_cancelBtn->setEnabled(true);
        });

        reposition();
        applyCardTheme();

        connect(&TM(), &ThemeManager::themeChanged, this, [this]() {
            applyCardTheme();
            update();
        });
    }

    void reposition()
    {
        if (QWidget *p = parentWidget()) {
            setGeometry(p->rect());
            if (auto *card = findChild<QFrame*>("updateCard"))
                card->move((width() - card->width()) / 2,
                           (height() - card->height()) / 2);
        }
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.fillRect(rect(), QColor(0, 0, 0, 180));
    }

    void resizeEvent(QResizeEvent *) override
    {
        reposition();
    }

private:
    UpdateChecker *m_checker;
    QString m_downloadUrl;
    QLabel *m_titleLabel = nullptr;
    QLabel *m_versionLabel = nullptr;
    QLabel *m_statusLabel = nullptr;
    QProgressBar *m_progressBar = nullptr;
    QPushButton *m_downloadBtn = nullptr;
    QPushButton *m_cancelBtn = nullptr;

    void applyCardTheme()
    {
        const auto &c = TM().colors();
        auto *card = findChild<QFrame*>("updateCard");
        if (!card) return;

        card->setStyleSheet(QStringLiteral(
            "#updateCard { background-color: %1; border: 1px solid %2; border-radius: %3; }")
            .arg(c.surface.name(), c.border.name())
            .arg(ThemeManager::radiusLarge));

        if (auto *tl = card->findChild<QLabel*>("titleLabel"))
            tl->setStyleSheet(QStringLiteral("color: %1; background: transparent;").arg(c.textPrimary.name()));
        if (auto *vl = card->findChild<QLabel*>("versionLabel"))
            vl->setStyleSheet(QStringLiteral("color: %1; background: transparent;").arg(c.textSecondary.name()));
        if (auto *sl = card->findChild<QLabel*>("statusLabel"))
            sl->setStyleSheet(QStringLiteral("color: %1; background: transparent;").arg(c.textSecondary.name()));

        if (auto *pb = card->findChild<QProgressBar*>("progressBar")) {
            pb->setStyleSheet(QStringLiteral(
                "QProgressBar { background-color: %1; border: 1px solid %2; border-radius: 6px; text-align: center; color: %3; }"
                "QProgressBar::chunk { background-color: %4; border-radius: 5px; }")
                .arg(c.surfaceAlt.name(), c.border.name(),
                     c.textSecondary.name(), c.accent.name()));
        }

        if (auto *updateBtn = card->findChild<QPushButton*>("updateBtn")) {
            updateBtn->setStyleSheet(QStringLiteral(
                "QPushButton { background-color: %1; color: %2; border: none; border-radius: 9px; padding: 0 20px; }"
                "QPushButton:hover { background-color: %3; }"
                "QPushButton:pressed { background-color: %4; }"
                "QPushButton:disabled { background-color: %5; color: %6; }")
                .arg(c.accent.name(), c.textOnAccent.name(),
                     c.accentHover.name(), c.accentPressed.name(),
                     c.surfaceAlt.name(), c.textTertiary.name()));
        }
        if (auto *quitBtn = card->findChild<QPushButton*>("quitBtn")) {
            quitBtn->setStyleSheet(QStringLiteral(
                "QPushButton { background-color: transparent; color: %1; border: 1px solid %2; border-radius: 9px; padding: 0 20px; }"
                "QPushButton:hover { background-color: %3; }"
                "QPushButton:pressed { background-color: %4; }"
                "QPushButton:disabled { color: %5; border-color: %5; }")
                .arg(c.textPrimary.name(), c.border.name(),
                     c.surfaceAlt.name(), c.surface.name(),
                     c.textTertiary.name()));
        }
    }
};

void UpdateChecker::showOverlay(QWidget *parentWindow,
                                const QString &latestVersion,
                                const QString &downloadUrl,
                                UpdateChecker *checker)
{
    auto *overlay = new UpdateOverlay(parentWindow, latestVersion, downloadUrl, checker);
    overlay->show();
    overlay->raise();
    overlay->setFocus();
    overlay->activateWindow();
}

bool UpdateChecker::isNewer(const QString &remote, const QString &local)
{
    QStringList r = parseVersion(remote);
    QStringList l = parseVersion(local);

    for (int i = 0; i < qMin(r.size(), l.size()); ++i) {
        int rv = r[i].toInt();
        int lv = l[i].toInt();
        if (rv > lv) return true;
        if (rv < lv) return false;
    }
    return r.size() > l.size();
}

QStringList UpdateChecker::parseVersion(const QString &v)
{
    return v.split('.', Qt::SkipEmptyParts);
}

#include "UpdateChecker.moc"
