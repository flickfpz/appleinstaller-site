#include "UpdateChecker.h"
#include "Theme.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkRequest>
#include <QDesktopServices>
#include <QUrl>
#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QPainter>
#include <QPainterPath>
#include <QGraphicsDropShadowEffect>

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

    emit updateRequired(remoteVer, url);
}

// ═════════════════════════════════════════════════════════════════════════════
// UpdateOverlay — full-screen blocking widget shown when update is required
// ═════════════════════════════════════════════════════════════════════════════

class UpdateOverlay : public QWidget
{
public:
    UpdateOverlay(QWidget *parent, const QString &latestVersion,
                  const QString &downloadUrl)
        : QWidget(parent)
    {
        setAttribute(Qt::WA_DeleteOnClose);
        setWindowFlags(Qt::Widget);
        setAttribute(Qt::WA_TranslucentBackground);
        raise();

        // ── Backdrop ──────────────────────────────────────────────────────

        // ── Center card ───────────────────────────────────────────────────
        auto *card = new QFrame(this);
        card->setObjectName("updateCard");
        card->setFixedSize(420, 320);

        auto *cardShadow = new QGraphicsDropShadowEffect(card);
        cardShadow->setBlurRadius(40);
        cardShadow->setOffset(0, 8);
        cardShadow->setColor(QColor(0, 0, 0, 100));
        card->setGraphicsEffect(cardShadow);

        auto *cardLayout = new QVBoxLayout(card);
        cardLayout->setContentsMargins(36, 32, 36, 28);
        cardLayout->setSpacing(0);

        // ── Icon / title ──────────────────────────────────────────────────
        auto *iconLabel = new QLabel(QStringLiteral("\u2B06"));
        iconLabel->setAlignment(Qt::AlignCenter);
        QFont iconFont = iconLabel->font();
        iconFont.setPixelSize(40);
        iconLabel->setFont(iconFont);
        cardLayout->addWidget(iconLabel);
        cardLayout->addSpacing(12);

        auto *titleLabel = new QLabel(QStringLiteral("Update Required"));
        titleLabel->setAlignment(Qt::AlignCenter);
        titleLabel->setFont(ThemeManager::fontTitle());
        titleLabel->setObjectName("titleLabel");
        cardLayout->addWidget(titleLabel);
        cardLayout->addSpacing(10);

        auto *versionLabel = new QLabel(
            QStringLiteral("You're running v%1 \u2014 v%2 is available")
                .arg(QApplication::applicationVersion(), latestVersion));
        versionLabel->setAlignment(Qt::AlignCenter);
        versionLabel->setWordWrap(true);
        versionLabel->setFont(ThemeManager::fontBody());
        versionLabel->setObjectName("versionLabel");
        cardLayout->addWidget(versionLabel);
        cardLayout->addSpacing(6);

        auto *subtitleLabel = new QLabel(
            QStringLiteral("Please update to continue using Rigset"));
        subtitleLabel->setAlignment(Qt::AlignCenter);
        subtitleLabel->setFont(ThemeManager::fontBody());
        subtitleLabel->setObjectName("subtitleLabel");
        cardLayout->addWidget(subtitleLabel);
        cardLayout->addStretch();

        // ── Buttons row ───────────────────────────────────────────────────
        auto *btnRow = new QHBoxLayout;
        btnRow->setSpacing(12);

        auto *quitBtn = new QPushButton(QStringLiteral("Quit"));
        quitBtn->setObjectName("quitBtn");
        quitBtn->setCursor(Qt::PointingHandCursor);
        quitBtn->setFixedHeight(38);
        quitBtn->setMinimumWidth(110);
        quitBtn->setFont(ThemeManager::fontHeading());

        auto *updateBtn = new QPushButton(QStringLiteral("Download Update"));
        updateBtn->setObjectName("updateBtn");
        updateBtn->setCursor(Qt::PointingHandCursor);
        updateBtn->setFixedHeight(38);
        updateBtn->setMinimumWidth(150);
        updateBtn->setFont(ThemeManager::fontHeading());
        updateBtn->setDefault(true);

        btnRow->addWidget(quitBtn);
        btnRow->addWidget(updateBtn);
        cardLayout->addLayout(btnRow);

        // ── Sizing ────────────────────────────────────────────────────────

        connect(updateBtn, &QPushButton::clicked, this, [downloadUrl]() {
            QDesktopServices::openUrl(QUrl(downloadUrl));
        });
        connect(quitBtn, &QPushButton::clicked, qApp, &QApplication::quit);

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
        if (auto *sl = card->findChild<QLabel*>("subtitleLabel"))
            sl->setStyleSheet(QStringLiteral("color: %1; background: transparent;").arg(c.textTertiary.name()));

        if (auto *updateBtn = card->findChild<QPushButton*>("updateBtn")) {
            updateBtn->setStyleSheet(QStringLiteral(
                "QPushButton { background-color: %1; color: %2; border: none; border-radius: 9px; padding: 0 20px; }"
                "QPushButton:hover { background-color: %3; }"
                "QPushButton:pressed { background-color: %4; }")
                .arg(c.accent.name(), c.textOnAccent.name(),
                     c.accentHover.name(), c.accentPressed.name()));
        }
        if (auto *quitBtn = card->findChild<QPushButton*>("quitBtn")) {
            quitBtn->setStyleSheet(QStringLiteral(
                "QPushButton { background-color: transparent; color: %1; border: 1px solid %2; border-radius: 9px; padding: 0 20px; }"
                "QPushButton:hover { background-color: %3; }"
                "QPushButton:pressed { background-color: %4; }")
                .arg(c.textPrimary.name(), c.border.name(),
                     c.surfaceAlt.name(), c.surface.name()));
        }
    }
};

void UpdateChecker::showOverlay(QWidget *parentWindow,
                                const QString &latestVersion,
                                const QString &downloadUrl)
{
    auto *overlay = new UpdateOverlay(parentWindow, latestVersion, downloadUrl);
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
