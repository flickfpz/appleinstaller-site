#pragma once
#include <QObject>
#include <QThread>
#include <QVector>
#include "AppData.h"

struct InstallResult {
    QString id;
    QString name;
    bool    success  = false;
    QString output;
    bool    skipped  = false;   // true when app not available on this OS
};

class InstallWorker : public QObject
{
    Q_OBJECT

public:
    explicit InstallWorker(const QVector<AppData> &apps, QObject *parent = nullptr);

public slots:
    void run();

signals:
    void started(int total);
    void progress(int current, int total, const QString &appName);
    void appFinished(const InstallResult &result);
    void finished(int succeeded, int failed, int skipped);

private:
    static InstallResult runCommand(const AppData &app);
    static bool isAlreadyInstalledCode(const QString &program, int code);

    QVector<AppData> m_apps;
};
