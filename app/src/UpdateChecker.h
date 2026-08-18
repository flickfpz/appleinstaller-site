#pragma once
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QString>

class UpdateChecker : public QObject
{
    Q_OBJECT
public:
    explicit UpdateChecker(QObject *parent = nullptr);

    void check();

signals:
    void updateAvailable(const QString &latestVersion, const QString &downloadUrl);

private slots:
    void onReplyFinished(QNetworkReply *reply);

private:
    QNetworkAccessManager m_nam;
    QString m_currentVersion;

    static bool isNewer(const QString &remote, const QString &local);
    static QStringList parseVersion(const QString &v);
};
