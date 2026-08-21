#pragma once
#include <QWidget>
#include <QLabel>
#include <QProgressBar>
#include <QListWidget>
#include <QPushButton>
#include <QPropertyAnimation>
#include "InstallWorker.h"

class ProgressOverlay : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)

public:
    explicit ProgressOverlay(QWidget *parent = nullptr);

    void startInstall(const QVector<AppData> &apps);
    void startUninstall(const QVector<AppData> &apps);
    void reset();

public slots:
    void applyTheme();

signals:
    void closed();

protected:
    void paintEvent(QPaintEvent *) override;
    void resizeEvent(QResizeEvent *) override;

private slots:
    void onStarted(int total);
    void onProgress(int current, int total, const QString &appName);
    void onAppFinished(const InstallResult &result);
    void onFinished(int succeeded, int failed, int skipped);

private:
    qreal opacity() const     { return m_opacity; }
    void  setOpacity(qreal v) { m_opacity = v; update(); }

    void beginRun(const QVector<AppData> &apps, InstallWorker::Mode mode);
    void fadeIn();
    void repositionCard();

    QWidget      *m_card        = nullptr;
    QLabel       *m_titleLabel  = nullptr;
    QLabel       *m_statusLabel = nullptr;
    QProgressBar *m_bar         = nullptr;
    QListWidget  *m_log         = nullptr;
    QPushButton  *m_doneBtn     = nullptr;

    QThread      *m_thread      = nullptr;
    InstallWorker*m_worker      = nullptr;

    bool          m_uninstall   = false;   // wording switches to "removing"
    qreal         m_opacity     = 0.0;
};
