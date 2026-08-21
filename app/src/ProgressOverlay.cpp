#include "ProgressOverlay.h"
#include "Theme.h"
#include "OsDetect.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QPainterPath>
#include <QThread>
#include <QPropertyAnimation>

ProgressOverlay::ProgressOverlay(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_TransparentForMouseEvents, false);
    setAutoFillBackground(false);

    // ── Card widget ───────────────────────────────────────────────────────────
    m_card = new QWidget(this);
    m_card->setFixedWidth(520);
    m_card->setAttribute(Qt::WA_StyledBackground, true);

    auto *cardLayout = new QVBoxLayout(m_card);
    cardLayout->setContentsMargins(36, 36, 36, 36);
    cardLayout->setSpacing(18);

    // Title
    m_titleLabel = new QLabel("Installing Apps", m_card);
    m_titleLabel->setFont(ThemeManager::fontTitle());
    m_titleLabel->setAlignment(Qt::AlignCenter);

    // Status
    m_statusLabel = new QLabel("Preparing…", m_card);
    m_statusLabel->setFont(ThemeManager::fontBody());
    m_statusLabel->setAlignment(Qt::AlignCenter);

    // Progress bar
    m_bar = new QProgressBar(m_card);
    m_bar->setRange(0, 100);
    m_bar->setValue(0);
    m_bar->setTextVisible(false);
    m_bar->setFixedHeight(6);

    // Log list
    m_log = new QListWidget(m_card);
    m_log->setFixedHeight(200);
    m_log->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_log->setFont(ThemeManager::fontBody());

    // Done button
    m_doneBtn = new QPushButton("Done", m_card);
    m_doneBtn->setFixedHeight(46);
    m_doneBtn->setFont(ThemeManager::fontHeading());
    m_doneBtn->setCursor(Qt::PointingHandCursor);
    m_doneBtn->hide();
    connect(m_doneBtn, &QPushButton::clicked, this, [this]() {
        hide();
        emit closed();
    });

    cardLayout->addWidget(m_titleLabel);
    cardLayout->addWidget(m_statusLabel);
    cardLayout->addWidget(m_bar);
    cardLayout->addWidget(m_log);
    cardLayout->addWidget(m_doneBtn);

    connect(&TM(), &ThemeManager::themeChanged, this, &ProgressOverlay::applyTheme);
    applyTheme();
}

// ── public ────────────────────────────────────────────────────────────────────

void ProgressOverlay::startInstall(const QVector<AppData> &apps)
{
    beginRun(apps, InstallWorker::Mode::Install);
    m_titleLabel->setText("Installing Apps");
}

void ProgressOverlay::startUninstall(const QVector<AppData> &apps)
{
    beginRun(apps, InstallWorker::Mode::Uninstall);
    m_titleLabel->setText("Removing Apps");
}

void ProgressOverlay::beginRun(const QVector<AppData> &apps, InstallWorker::Mode mode)
{
    m_uninstall = (mode == InstallWorker::Mode::Uninstall);
    m_log->clear();
    m_bar->setValue(0);
    m_doneBtn->hide();
    m_statusLabel->setText("Preparing…");

    show();
    raise();
    repositionCard();
    fadeIn();

    if (m_thread) {
        m_thread->quit();
        m_thread->wait();
        delete m_thread;
        m_thread = nullptr;
    }

    m_thread = new QThread(this);
    m_worker = new InstallWorker(apps, mode);
    m_worker->moveToThread(m_thread);

    connect(m_thread, &QThread::started,     m_worker, &InstallWorker::run);
    connect(m_worker, &InstallWorker::started,     this, &ProgressOverlay::onStarted);
    connect(m_worker, &InstallWorker::progress,    this, &ProgressOverlay::onProgress);
    connect(m_worker, &InstallWorker::appFinished, this, &ProgressOverlay::onAppFinished);
    connect(m_worker, &InstallWorker::finished,    this, &ProgressOverlay::onFinished);
    connect(m_worker, &InstallWorker::finished, m_thread, [this](int,int,int){ m_thread->quit(); });
    connect(m_thread, &QThread::finished,      m_worker, &QObject::deleteLater);

    m_thread->start();
}

void ProgressOverlay::reset()
{
    if (m_thread) { m_thread->quit(); m_thread->wait(); }
    hide();
}

// ── applyTheme ────────────────────────────────────────────────────────────────

void ProgressOverlay::applyTheme()
{
    auto c = [](const QColor &col) { return col.name(QColor::HexArgb); };
    const auto &pal   = TM().colors();
    const bool  isMid = pal.hasGradientBg;

    // Card background
    if (isMid) {
        m_card->setStyleSheet(QStringLiteral(
            "QWidget { background-color: rgba(22,16,50,245); border-radius: 22px; }"));
    } else {
        m_card->setStyleSheet(QStringLiteral(
            "QWidget { background-color: %1; border-radius: 22px; }")
            .arg(c(pal.surface)));
    }

    // Labels
    auto setPal = [](QLabel *lbl, const QColor &col) {
        QPalette p = lbl->palette();
        p.setColor(QPalette::WindowText, col);
        lbl->setPalette(p);
    };
    setPal(m_titleLabel,  pal.textPrimary);
    setPal(m_statusLabel, pal.textSecondary);

    // Progress bar
    if (isMid) {
        m_bar->setStyleSheet(QStringLiteral(R"(
QProgressBar {
    background-color: rgba(50,38,100,180);
    border-radius: 4px;
    border: none;
}
QProgressBar::chunk {
    background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
        stop:0 #8A50FF, stop:0.5 #6060FF, stop:1 #40B0FF);
    border-radius: 4px;
})"));
    } else {
        m_bar->setStyleSheet(QStringLiteral(R"(
QProgressBar {
    background-color: %1;
    border-radius: 3px;
    border: none;
}
QProgressBar::chunk {
    background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
        stop:0 %2, stop:1 %3);
    border-radius: 3px;
})")
        .arg(c(pal.surfaceAlt))
        .arg(c(pal.accent))
        .arg(c(pal.accentHover)));
    }

    // Log list
    m_log->setStyleSheet(QStringLiteral(R"(
QListWidget {
    background-color: %1;
    border: 1px solid %2;
    border-radius: 12px;
    padding: 6px;
    font-size: 13px;
    color: %3;
}
QListWidget::item { padding: 3px 4px; })")
    .arg(c(pal.surfaceAlt))
    .arg(c(pal.border))
    .arg(c(pal.textPrimary)));

    // Done button
    if (isMid) {
        m_doneBtn->setStyleSheet(QStringLiteral(R"(
QPushButton {
    background: qlineargradient(x1:0,y1:0,x2:1,y2:1,
        stop:0 #8A50FF, stop:1 #5028D0);
    color: #FFFFFF;
    border: none;
    border-radius: 14px;
    font-size: 15px;
    font-weight: 700;
}
QPushButton:hover {
    background: qlineargradient(x1:0,y1:0,x2:1,y2:1,
        stop:0 #A060FF, stop:1 #6038F0);
}
QPushButton:pressed {
    background: qlineargradient(x1:0,y1:0,x2:1,y2:1,
        stop:0 #7040E0, stop:1 #4020C0);
}
)"));
    } else {
        m_doneBtn->setStyleSheet(QStringLiteral(R"(
QPushButton {
    background-color: %1;
    color: %2;
    border: none;
    border-radius: 14px;
    font-size: 15px;
    font-weight: 700;
}
QPushButton:hover   { background-color: %3; }
QPushButton:pressed { background-color: %4; })")
        .arg(c(pal.accent))
        .arg(c(pal.accentText))
        .arg(c(pal.accentHover))
        .arg(c(pal.accentPressed)));
    }

    update();
}

// ── slots ─────────────────────────────────────────────────────────────────────

void ProgressOverlay::onStarted(int /*total*/) { m_bar->setValue(0); }

void ProgressOverlay::onProgress(int current, int total, const QString &appName)
{
    int pct = total > 0 ? int(current * 100.0 / total) : 0;
    m_bar->setValue(pct);
    if (!appName.isEmpty())
        m_statusLabel->setText(
            QStringLiteral("%1 %2…  (%3 of %4)")
            .arg(m_uninstall ? "Removing" : "Installing",
                 appName, QString::number(current + 1), QString::number(total)));
    else
        m_statusLabel->setText("Finishing up…");
}

void ProgressOverlay::onAppFinished(const InstallResult &result)
{
    QString icon, text;
    if (result.skipped) {
        icon = "⊘";
        text = m_uninstall
            ? QStringLiteral("%1  %2  (nothing to remove on %3)")
                       .arg(icon, result.name, OsDetect::tag())
            : QStringLiteral("%1  %2  (not available on %3)")
                       .arg(icon, result.name, OsDetect::tag());
        auto *item = new QListWidgetItem(text, m_log);
        item->setForeground(TM().colors().textTertiary);
    } else {
        icon = result.success ? "✓" : "✗";
        auto *item = new QListWidgetItem(
            QStringLiteral("%1  %2").arg(icon, result.name), m_log);
        item->setForeground(result.success ? TM().success() : TM().danger());
    }
    m_log->scrollToBottom();
}

void ProgressOverlay::onFinished(int succeeded, int failed, int skipped)
{
    m_bar->setValue(100);
    m_statusLabel->setText(
        QStringLiteral("Done — %1 %2, %3 failed, %4 skipped")
            .arg(succeeded)
            .arg(m_uninstall ? "removed" : "installed")
            .arg(failed).arg(skipped));
    m_titleLabel->setText(failed == 0 ? (m_uninstall ? "All Removed" : "All Done ")
                                      : "Completed with Errors");
    m_doneBtn->show();
}

// ── paint / layout ────────────────────────────────────────────────────────────

void ProgressOverlay::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const auto &pal   = TM().colors();
    const bool  isMid = pal.hasGradientBg;

    if (isMid) {
        // Deep cosmic backdrop for midnight
        QLinearGradient grad(0, 0, width(), height());
        grad.setColorAt(0.0, QColor( 4,  2, 18, int(230 * m_opacity)));
        grad.setColorAt(0.5, QColor( 8,  4, 28, int(220 * m_opacity)));
        grad.setColorAt(1.0, QColor( 6,  2, 22, int(225 * m_opacity)));
        p.fillRect(rect(), grad);

        // Purple nebula center glow
        QRadialGradient nebula(rect().center(), qMin(width(), height()) * 0.55);
        nebula.setColorAt(0.0, QColor( 80, 40, 160, int(60 * m_opacity)));
        nebula.setColorAt(0.6, QColor( 40, 20,  80, int(30 * m_opacity)));
        nebula.setColorAt(1.0, Qt::transparent);
        p.fillRect(rect(), nebula);
    } else {
        // Simple dim for Light/Dark
        const QColor &dim = pal.dimOverlay;
        QColor bg = dim;
        bg.setAlpha(int(dim.alpha() * m_opacity));
        p.fillRect(rect(), bg);
    }

    // Soft vignette ring
    QRadialGradient vign(rect().center(), qMin(width(), height()) * 0.72);
    vign.setColorAt(0, Qt::transparent);
    vign.setColorAt(1, QColor(0, 0, 0, int(80 * m_opacity)));
    p.fillRect(rect(), vign);
}

void ProgressOverlay::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);
    repositionCard();
}

// ── private ───────────────────────────────────────────────────────────────────

void ProgressOverlay::fadeIn()
{
    auto *anim = new QPropertyAnimation(this, "opacity", this);
    anim->setDuration(250);
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void ProgressOverlay::repositionCard()
{
    if (m_card) {
        m_card->adjustSize();
        m_card->move((width()  - m_card->width())  / 2,
                     (height() - m_card->height()) / 2);
    }
}
