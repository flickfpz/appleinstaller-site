#include "WizardPage.h"
#include "Theme.h"
#include "OsDetect.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QPainter>
#include <QPainterPath>

WizardPage::WizardPage(const QString &title, const QVector<AppData> &apps,
                       QWidget *parent)
    : QWidget(parent), m_title(title)
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    auto *header = new QWidget(this);
    header->setFixedHeight(110);
    header->setAttribute(Qt::WA_StyledBackground, true);

    auto *hLayout = new QVBoxLayout(header);
    hLayout->setContentsMargins(28, 20, 28, 10);
    hLayout->setSpacing(8);

    m_titleLabel = new QLabel(m_title, header);
    m_titleLabel->setFont(ThemeManager::fontTitle());

    auto *subtitle = new QLabel("Pick the apps you want from this category.", header);
    subtitle->setFont(ThemeManager::fontBody());

    hLayout->addWidget(m_titleLabel);
    hLayout->addWidget(subtitle);
    root->addWidget(header);

    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setAttribute(Qt::WA_TranslucentBackground);
    m_scrollArea->viewport()->setAttribute(Qt::WA_TranslucentBackground);
    root->addWidget(m_scrollArea, 1);

    m_gridContainer = new QWidget;
    m_gridContainer->setAttribute(Qt::WA_TranslucentBackground);
    m_grid = new QGridLayout(m_gridContainer);
    m_grid->setContentsMargins(24, 16, 24, 24);
    m_grid->setSpacing(ThemeManager::cardSpacing);
    m_scrollArea->setWidget(m_gridContainer);

    buildGrid(apps);

    auto *bottomBar = new QWidget(this);
    bottomBar->setFixedHeight(72);
    bottomBar->setAttribute(Qt::WA_StyledBackground, true);

    auto *bottomH = new QHBoxLayout(bottomBar);
    bottomH->setContentsMargins(24, 14, 24, 14);
    bottomH->setSpacing(10);

    m_countLabel = new QLabel("No apps selected", bottomBar);
    m_countLabel->setFont(ThemeManager::fontBody());

    m_selectAllBtn = new QPushButton("Select All", bottomBar);
    m_selectAllBtn->setFixedHeight(34);
    m_selectAllBtn->setCursor(Qt::PointingHandCursor);
    m_selectAllBtn->setFont(ThemeManager::fontBody());
    connect(m_selectAllBtn, &QPushButton::clicked, this, &WizardPage::selectAll);

    m_deselectBtn = new QPushButton("Deselect All", bottomBar);
    m_deselectBtn->setFixedHeight(34);
    m_deselectBtn->setCursor(Qt::PointingHandCursor);
    m_deselectBtn->setFont(ThemeManager::fontBody());
    connect(m_deselectBtn, &QPushButton::clicked, this, &WizardPage::deselectAll);

    m_backBtn = new QPushButton("Back", bottomBar);
    m_backBtn->setFixedHeight(34);
    m_backBtn->setCursor(Qt::PointingHandCursor);
    m_backBtn->setFont(ThemeManager::fontBody());
    connect(m_backBtn, &QPushButton::clicked, this, &WizardPage::backClicked);

    m_nextBtn = new QPushButton("Next", bottomBar);
    m_nextBtn->setFixedHeight(46);
    m_nextBtn->setMinimumWidth(120);
    m_nextBtn->setCursor(Qt::PointingHandCursor);
    m_nextBtn->setFont(ThemeManager::fontHeading());
    connect(m_nextBtn, &QPushButton::clicked, this, &WizardPage::nextClicked);

    m_installBtn = new QPushButton("Install Selected", bottomBar);
    m_installBtn->setFixedHeight(46);
    m_installBtn->setMinimumWidth(175);
    m_installBtn->setCursor(Qt::PointingHandCursor);
    m_installBtn->setFont(ThemeManager::fontHeading());
    m_installBtn->setEnabled(false);
    connect(m_installBtn, &QPushButton::clicked, this, &WizardPage::installClicked);

    bottomH->addWidget(m_countLabel, 1);
    bottomH->addWidget(m_selectAllBtn);
    bottomH->addWidget(m_deselectBtn);
    bottomH->addWidget(m_backBtn);
    bottomH->addWidget(m_nextBtn);
    bottomH->addWidget(m_installBtn);
    root->addWidget(bottomBar);

    connect(&TM(), &ThemeManager::themeChanged, this, &WizardPage::applyTheme);
    applyTheme();
}

void WizardPage::buildGrid(const QVector<AppData> &apps)
{
    for (const AppData &app : apps) {
        auto *card = new AppCard(app, m_gridContainer);
        card->setAvailable(app.availableOnCurrentOs());
        m_cards[app.id] = card;
        connect(card, &AppCard::selectionChanged, this, [this](const QString &, bool) {
            int n = selectedIds().size();
            m_countLabel->setText(n == 0 ? "No apps selected"
                                 : n == 1 ? "1 app selected"
                                          : QStringLiteral("%1 apps selected").arg(n));
            m_installBtn->setEnabled(n > 0);
            m_installBtn->setText(n > 0
                ? QStringLiteral("Install %1 App%2").arg(n).arg(n > 1 ? "s" : "")
                : "Install Selected");
            emit selectionChanged();
        });
    }

    int availW = m_scrollArea->viewport()->width() - 48;
    int cols   = qMax(1, availW / (218 + ThemeManager::cardSpacing));

    int row = 0, col = 0;
    for (auto it = m_cards.begin(); it != m_cards.end(); ++it) {
        it.value()->show();
        m_grid->addWidget(it.value(), row, col);
        if (++col >= cols) { col = 0; ++row; }
    }
    m_grid->setRowStretch(row + 1, 1);
    m_grid->setColumnStretch(cols, 1);
}

QVector<QString> WizardPage::selectedIds() const
{
    QVector<QString> ids;
    for (auto it = m_cards.begin(); it != m_cards.end(); ++it)
        if (it.value()->isChecked())
            ids << it.key();
    return ids;
}

void WizardPage::selectAll()
{
    for (auto it = m_cards.begin(); it != m_cards.end(); ++it)
        if (!it.value()->isHidden() && it.value()->isAvailable())
            it.value()->setChecked(true);
}

void WizardPage::deselectAll()
{
    for (auto it = m_cards.begin(); it != m_cards.end(); ++it)
        it.value()->setChecked(false);
}

void WizardPage::applyTheme()
{
    auto c = [](const QColor &col) { return col.name(QColor::HexArgb); };
    const auto &pal   = TM().colors();
    const bool  isMid = pal.hasGradientBg;

    if (isMid) {
        setStyleSheet("QWidget { background: transparent; }");
    } else {
        setStyleSheet(QStringLiteral("QWidget { background-color: %1; }").arg(c(pal.bg)));
    }

    auto *header = m_titleLabel->parentWidget();
    if (header) {
        if (isMid) {
            header->setStyleSheet(
                "QWidget { background-color: rgba(14,10,36,215); "
                "border-bottom: 1px solid rgba(100,70,180,160); }");
        } else {
            header->setStyleSheet(QStringLiteral(
                "QWidget { background-color: %1; border-bottom: 1px solid %2; }")
                .arg(c(pal.glassBg)).arg(c(pal.glassBorder)));
        }
    }

    auto setPal = [](QLabel *lbl, const QColor &col) {
        QPalette p = lbl->palette();
        p.setColor(QPalette::WindowText, col);
        lbl->setPalette(p);
    };
    setPal(m_titleLabel, pal.textPrimary);
    setPal(m_countLabel, pal.textSecondary);

    auto *bottomBar = qobject_cast<QWidget *>(m_countLabel->parentWidget());
    if (bottomBar) {
        if (isMid) {
            bottomBar->setStyleSheet(
                "QWidget { background-color: rgba(12,8,32,220); "
                "border-top: 1px solid rgba(90,60,160,140); }");
        } else {
            bottomBar->setStyleSheet(QStringLiteral(
                "QWidget { background-color: %1; border-top: 1px solid %2; }")
                .arg(c(pal.glassBg)).arg(c(pal.glassBorder)));
        }
    }

    if (isMid) {
        m_selectAllBtn->setStyleSheet(QStringLiteral(R"(
QPushButton {
    background: transparent; border: 1px solid rgba(138,80,255,180);
    border-radius: 10px; color: rgba(180,140,255,255); padding: 0 14px; font-size: 13px;
}
QPushButton:hover { background-color: rgba(138,80,255,30); }
QPushButton:pressed { background-color: rgba(138,80,255,55); }
)"));
    } else {
        m_selectAllBtn->setStyleSheet(QStringLiteral(R"(
QPushButton {
    background: transparent; border: 1px solid %1; border-radius: 10px;
    color: %1; padding: 0 14px; font-size: 13px;
}
QPushButton:hover { background-color: %2; }
QPushButton:pressed { background-color: %3; }
)").arg(c(pal.accent)).arg(c(pal.accentSubtle)).arg(c(pal.cardSelected)));
    }

    m_deselectBtn->setStyleSheet(QStringLiteral(R"(
QPushButton {
    background: transparent; border: 1px solid %1; border-radius: 10px;
    color: %2; padding: 0 14px; font-size: 13px;
}
QPushButton:hover { background-color: %3; }
QPushButton:pressed { background-color: %4; }
)").arg(c(pal.border)).arg(c(pal.textSecondary)).arg(c(pal.surfaceAlt)).arg(c(pal.surfaceElevated)));

    if (isMid) {
        m_backBtn->setStyleSheet(QStringLiteral(R"(
QPushButton {
    background: transparent; border: 1px solid rgba(138,80,255,120);
    border-radius: 10px; color: rgba(180,140,255,255); padding: 0 14px; font-size: 13px;
}
QPushButton:hover { background-color: rgba(138,80,255,25); }
)"));
        m_nextBtn->setStyleSheet(QStringLiteral(R"(
QPushButton {
    background: qlineargradient(x1:0,y1:0,x2:1,y2:1,
        stop:0 #8A50FF, stop:0.5 #6A3AEF, stop:1 #5028D0);
    color: #FFFFFF; border: none; border-radius: 14px;
    font-size: 14px; font-weight: 700; padding: 0 28px;
}
QPushButton:hover {
    background: qlineargradient(x1:0,y1:0,x2:1,y2:1,
        stop:0 #A060FF, stop:0.5 #8050FF, stop:1 #6038F0);
}
)"));
        m_installBtn->setStyleSheet(QStringLiteral(R"(
QPushButton {
    background: qlineargradient(x1:0,y1:0,x2:1,y2:1,
        stop:0 #8A50FF, stop:0.5 #6A3AEF, stop:1 #5028D0);
    color: #FFFFFF; border: none; border-radius: 14px;
    font-size: 14px; font-weight: 700; padding: 0 28px;
}
QPushButton:hover {
    background: qlineargradient(x1:0,y1:0,x2:1,y2:1,
        stop:0 #A060FF, stop:0.5 #8050FF, stop:1 #6038F0);
}
QPushButton:disabled {
    background: rgba(60,50,100,180); color: rgba(150,140,200,130);
}
)"));
    } else {
        m_backBtn->setStyleSheet(QStringLiteral(R"(
QPushButton {
    background: transparent; border: 1px solid %1; border-radius: 10px;
    color: %2; padding: 0 14px; font-size: 13px;
}
QPushButton:hover { background-color: %3; }
)").arg(c(pal.border)).arg(c(pal.textSecondary)).arg(c(pal.surfaceAlt)));

        m_nextBtn->setStyleSheet(QStringLiteral(R"(
QPushButton {
    background-color: %1; color: %2; border: none; border-radius: 14px;
    font-size: 14px; font-weight: 700; padding: 0 28px;
}
QPushButton:hover { background-color: %3; }
QPushButton:pressed { background-color: %4; }
)").arg(c(pal.accent)).arg(c(pal.accentText)).arg(c(pal.accentHover)).arg(c(pal.accentPressed)));

        m_installBtn->setStyleSheet(QStringLiteral(R"(
QPushButton {
    background-color: %1; color: %2; border: none; border-radius: 14px;
    font-size: 14px; font-weight: 700; padding: 0 28px;
}
QPushButton:hover { background-color: %3; }
QPushButton:pressed { background-color: %4; }
QPushButton:disabled { background-color: %5; color: %6; }
)").arg(c(pal.accent)).arg(c(pal.accentText)).arg(c(pal.accentHover))
          .arg(c(pal.accentPressed)).arg(c(pal.accentDisabled)).arg(c(pal.textDisabled)));
    }

    m_gridContainer->setStyleSheet("background: transparent;");
}

void WizardPage::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const auto &pal  = TM().colors();
    const bool  isMid = pal.hasGradientBg;

    if (!isMid) return;

    QPainterPath path;
    path.addRoundedRect(rect(), 0, 0);

    QLinearGradient grad(0, 0, 0, height());
    grad.setColorAt(0.0, pal.gradA);
    grad.setColorAt(0.5, pal.gradB);
    grad.setColorAt(1.0, pal.gradC);
    p.setBrush(grad);
    p.setPen(Qt::NoPen);
    p.drawRect(rect());
}

void WizardPage::setBackVisible(bool v) { m_backBtn->setVisible(v); }
void WizardPage::setNextVisible(bool v) { m_nextBtn->setVisible(v); }
