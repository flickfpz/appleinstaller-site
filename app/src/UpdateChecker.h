#pragma once
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QString>

class QWidget;

class UpdateChecker : public QObject
{
    Q_OBJECT
public:
    explicit UpdateChecker(QObject *parent = nullptr);

    void check();

    static void showOverlay(QWidget *parentWindow,
                            const QString &latestVersion,
                            const QString &downloadUrl,
                            UpdateChecker *checker);

signals:
    void updateRequired(const QString &latestVersion, const QString &downloadUrl);
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void downloadFinished(const QString &filePath);
    void downloadFailed(const QString &error);
    void statusChanged(const QString &status);

private slots:
    void onVersionReplyFinished(QNetworkReply *reply);
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void onDownloadFinished();
    void onDownloadError(QNetworkReply::NetworkError error);

public slots:
    void startDownload(const QString &url);
    void cancelDownload();
    void applyUpdate(const QString &downloadedFile);

private:
    QNetworkAccessManager m_nam;
    QString m_currentVersion;
    QNetworkReply *m_downloadReply = nullptr;

    static bool isNewer(const QString &remote, const QString &local);
    static QStringList parseVersion(const QString &v);
};
