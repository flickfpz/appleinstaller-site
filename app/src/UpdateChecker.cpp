#include "UpdateChecker.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkRequest>
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>
#include <QApplication>
#include <QStyle>

static const QString VERSION_URL =
    QStringLiteral("https://flickfpz.github.io/appleinstaller-site/version.json");

UpdateChecker::UpdateChecker(QObject *parent)
    : QObject(parent)
    , m_currentVersion(QApplication::applicationVersion())
{
    connect(&m_nam, &QNetworkAccessManager::finished,
            this, &UpdateChecker::onReplyFinished);
}

void UpdateChecker::check()
{
    QNetworkRequest req(QUrl(VERSION_URL));
    req.setTransferTimeout(5000);
    m_nam.get(req);
}

void UpdateChecker::onReplyFinished(QNetworkReply *reply)
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

    // Determine platform download URL
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

    emit updateAvailable(remoteVer, url);

    // Show dialog on the main thread
    QMessageBox msg;
    msg.setIcon(QMessageBox::Information);
    msg.setWindowTitle("Update Available");
    msg.setText("A new version (" + remoteVer + ") is available.");
    msg.setInformativeText("You are running version " + m_currentVersion + ".\nWould you like to download the update?");
    msg.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msg.setDefaultButton(QMessageBox::Yes);
    msg.style()->polish(&msg);

    if (msg.exec() == QMessageBox::Yes) {
        QDesktopServices::openUrl(QUrl(url));
    }
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
