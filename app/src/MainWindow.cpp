#include "MainWindow.h"
#include "Theme.h"
#include "OsDetect.h"

#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QPainter>
#include <QPainterPath>
#include <QLinearGradient>
#include <QRadialGradient>
#include <QResizeEvent>
#include <QScreen>
#include <QPropertyAnimation>

// ═════════════════════════════════════════════════════════════════════════════
// GradientBackground
// ═════════════════════════════════════════════════════════════════════════════

class GradientBackground : public QWidget
{
    Q_OBJECT
public:
    explicit GradientBackground(QWidget *parent = nullptr) : QWidget(parent)
    {
        setAttribute(Qt::WA_TransparentForMouseEvents);
        setAttribute(Qt::WA_NoSystemBackground);
        lower();
        connect(&TM(), &ThemeManager::themeChanged, this, [this]{ update(); });
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        const auto &pal = TM().colors();
        if (!pal.hasGradientBg) {
            QPainter p(this);
            p.fillRect(rect(), pal.bg);
            return;
        }

        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);

        QLinearGradient grad(0, 0, width(), height());
        grad.setColorAt(0.0, pal.gradA);
        grad.setColorAt(0.45, pal.gradB);
        grad.setColorAt(1.0, pal.gradC);
        p.fillRect(rect(), grad);

        QRadialGradient nebulaA(width() * 0.82, height() * 0.12, width() * 0.45);
        nebulaA.setColorAt(0.0, QColor(110, 50, 200, 55));
        nebulaA.setColorAt(1.0, Qt::transparent);
        p.fillRect(rect(), nebulaA);

        QRadialGradient nebulaB(width() * 0.15, height() * 0.88, width() * 0.4);
        nebulaB.setColorAt(0.0, QColor(40, 60, 160, 45));
        nebulaB.setColorAt(1.0, Qt::transparent);
        p.fillRect(rect(), nebulaB);

        p.setPen(Qt::NoPen);
        const int dotCount = 28;
        const quint64 seed = quint64(width()) * 31337 + quint64(height());
        for (int i = 0; i < dotCount; ++i) {
            quint64 h = seed ^ quint64(i * 2654435761ULL);
            h ^= h >> 33; h *= 0xff51afd7ed558ccdULL; h ^= h >> 33;
            int x   = int(h % quint64(width()));
            int y   = int((h >> 20) % quint64(height()));
            int sz  = int((h >> 40) % 3) + 1;
            int a   = int((h >> 48) % 80) + 20;
            p.setBrush(QColor(220, 210, 255, a));
            p.drawEllipse(x, y, sz, sz);
        }
    }
};

#include "MainWindow.moc"

// ═════════════════════════════════════════════════════════════════════════════
// ThemePicker
// ═════════════════════════════════════════════════════════════════════════════

ThemePicker::ThemePicker(QWidget *parent) : QWidget(parent)
{
    setFixedHeight(38);
    setAttribute(Qt::WA_StyledBackground, true);

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(5, 5, 5, 5);
    layout->setSpacing(2);

    struct Def { ThemeManager::Theme t; QString lbl; QColor dot; };
    const QVector<Def> defs = {
        { ThemeManager::Theme::Light,     "Light",     { 255, 210,  60 } },
        { ThemeManager::Theme::Dark,      "Dark",      {  70, 140, 255 } },
        { ThemeManager::Theme::Midnight,  "Midnight",  { 150,  80, 255 } },
        { ThemeManager::Theme::TrueBlack, "True Black",{   0,   0,   0 } },
    };

    for (const Def &d : defs) {
        auto *btn = new QPushButton(d.lbl, this);
        btn->setCheckable(true);
        btn->setFixedHeight(28);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setFont(ThemeManager::fontCaption());
        btn->setProperty("themeId", int(d.t));
        layout->addWidget(btn);
        m_swatches.append(Swatch(btn, d.t, d.dot, d.lbl));

        connect(btn, &QPushButton::clicked, this, [this, d]() {
            TM().setTheme(d.t);
            updateActive();
        });
    }

    updateActive();
    connect(&TM(), &ThemeManager::themeChanged, this, &ThemePicker::applyTheme);
    applyTheme();
}

void ThemePicker::updateActive()
{
    for (auto &sw : m_swatches)
        sw.btn->setChecked(sw.themeId == TM().currentTheme());
}

void ThemePicker::applyTheme()
{
    updateActive();

    auto c = [](const QColor &col) { return col.name(QColor::HexArgb); };
    const auto &pal   = TM().colors();
    const bool  isMid = pal.hasGradientBg;

    if (isMid) {
        setStyleSheet(
            "QWidget { background-color: rgba(30,20,60,200); border-radius: 12px; }");
    } else {
        setStyleSheet(QStringLiteral(
            "QWidget { background-color: %1; border-radius: 12px; }")
            .arg(c(pal.surfaceAlt)));
    }

    for (const auto &sw : m_swatches) {
        bool active = (sw.themeId == TM().currentTheme());

        if (isMid && active) {
            sw.btn->setStyleSheet(QStringLiteral(R"(
QPushButton {
    background: qlineargradient(x1:0,y1:0,x2:1,y2:1,
        stop:0 rgba(120,60,220,200), stop:1 rgba(80,40,180,200));
    color: #FFFFFF; border: 1px solid rgba(180,100,255,150);
    border-radius: 9px; padding: 0 10px; font-size: 11px; font-weight: 600;
}
QPushButton:hover {
    background: qlineargradient(x1:0,y1:0,x2:1,y2:1,
        stop:0 rgba(150,80,255,220), stop:1 rgba(100,60,200,220));
}
)"));
        } else if (active) {
            sw.btn->setStyleSheet(QStringLiteral(R"(
QPushButton {
    background: %1; color: %2; border: none; border-radius: 9px;
    padding: 0 10px; font-size: 11px; font-weight: 600;
}
QPushButton:hover { background: %3; }
)").arg(c(pal.surface)).arg(c(pal.textPrimary)).arg(c(pal.surfaceElevated)));
        } else {
            sw.btn->setStyleSheet(QStringLiteral(R"(
QPushButton {
    background: transparent; color: %1; border: none; border-radius: 9px;
    padding: 0 10px; font-size: 11px; font-weight: 400;
}
QPushButton:hover { background-color: %2; }
)").arg(c(pal.textSecondary)).arg(c(pal.accentSubtle)));
        }
    }
    update();
}

void ThemePicker::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    for (const auto &sw : m_swatches) {
        QRect br = sw.btn->geometry();
        int dotX = br.left() + 10;
        int dotY = br.center().y();
        int dotR = 4;

        QColor col = sw.dot;
        bool active = (sw.themeId == TM().currentTheme());
        if (!active) col.setAlpha(140);

        p.setPen(Qt::NoPen);
        p.setBrush(col);
        p.drawEllipse(dotX, dotY - dotR, dotR * 2, dotR * 2);

        // White outline for dark dots so they stay visible
        if (col.lightness() < 40) {
            p.setBrush(Qt::NoBrush);
            p.setPen(QPen(QColor(120, 120, 120), 1.0));
            p.drawEllipse(dotX, dotY - dotR, dotR * 2, dotR * 2);
            p.setPen(Qt::NoPen);
        }

        if (active) {
            QRadialGradient glow(dotX + dotR, dotY, dotR * 3);
            glow.setColorAt(0, QColor(col.red(), col.green(), col.blue(), 80));
            glow.setColorAt(1, Qt::transparent);
            p.setBrush(glow);
            p.drawEllipse(dotX - dotR * 2, dotY - dotR * 3, dotR * 6, dotR * 6);
        }
    }
}

// ═════════════════════════════════════════════════════════════════════════════
// MainWindow
// ═════════════════════════════════════════════════════════════════════════════

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    setWindowTitle("Rigset");
    setMinimumSize(960, 660);
    resize(1180, 760);

    m_catalogue = defaultCatalogue();

    QStringList categories;
    for (const AppData &a : m_catalogue)
        if (!categories.contains(a.category))
            categories << a.category;
    m_categoryOrder = categories;

    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);

    m_gradientBg = new GradientBackground(m_centralWidget);
    m_gradientBg->setGeometry(m_centralWidget->rect());

    auto *rootH = new QHBoxLayout(m_centralWidget);
    rootH->setContentsMargins(0, 0, 0, 0);
    rootH->setSpacing(0);

    m_sidebar = new CategoryPanel(categories, m_centralWidget);
    connect(m_sidebar, &CategoryPanel::categorySelected, this, [this](const QString &cat) {
        if (m_wizardMode) {
            if (cat.isEmpty()) {
                goToPage(0);
            } else {
                int idx = m_categoryOrder.indexOf(cat);
                if (idx >= 0) goToPage(idx + 1);
            }
        } else {
            filterGrid(cat);
        }
    });
    rootH->addWidget(m_sidebar);

    auto *rightPane = new QWidget(m_centralWidget);
    rightPane->setAttribute(Qt::WA_TranslucentBackground);
    auto *rightV = new QVBoxLayout(rightPane);
    rightV->setContentsMargins(0, 0, 0, 0);
    rightV->setSpacing(0);
    rootH->addWidget(rightPane, 1);

    m_headerWidget = new QWidget(rightPane);
    m_headerWidget->setFixedHeight(126);
    m_headerWidget->setAttribute(Qt::WA_StyledBackground, true);

    auto *hLayout = new QVBoxLayout(m_headerWidget);
    hLayout->setContentsMargins(28, 20, 28, 14);
    hLayout->setSpacing(6);

    m_titleLabel = new QLabel("Rigset", m_headerWidget);
    m_titleLabel->setFont(ThemeManager::fontTitle());

    m_subtitleLabel = new QLabel(
        "Pick the apps you want — we'll install them all at once.",
        m_headerWidget);
    m_subtitleLabel->setFont(ThemeManager::fontBody());

    m_stepLabel = new QLabel(m_headerWidget);
    m_stepLabel->setFont(ThemeManager::fontCaption());

    m_searchBox = new QLineEdit(m_headerWidget);
    m_searchBox->setPlaceholderText("Search apps...");
    m_searchBox->setFixedHeight(32);
    m_searchBox->setMaximumWidth(240);
    m_searchBox->hide();

    m_themePicker = new ThemePicker(m_headerWidget);

    m_wizardBtn = new QPushButton(m_headerWidget);
    m_wizardBtn->setFixedHeight(30);
    m_wizardBtn->setCursor(Qt::PointingHandCursor);
    m_wizardBtn->setFont(ThemeManager::fontCaption());

    m_osBadge = new QLabel(m_headerWidget);
    m_osBadge->setFont(ThemeManager::fontCaption());
    m_osBadge->setToolTip(OsDetect::name());

    auto *topRow = new QHBoxLayout;
    topRow->setContentsMargins(0, 0, 0, 0);
    topRow->setSpacing(10);
    topRow->addWidget(m_titleLabel);
    topRow->addSpacing(6);
    topRow->addWidget(m_osBadge);
    topRow->addStretch();
    topRow->addWidget(m_wizardBtn);
    topRow->addWidget(m_themePicker);
    topRow->addWidget(m_searchBox);

    hLayout->addLayout(topRow);
    hLayout->addWidget(m_subtitleLabel);
    hLayout->addWidget(m_stepLabel);

    rightV->addWidget(m_headerWidget);

    m_stack = new QStackedWidget(rightPane);
    rightV->addWidget(m_stack, 1);

    m_bottomBar = new QWidget(rightPane);
    m_bottomBar->setFixedHeight(72);
    m_bottomBar->setAttribute(Qt::WA_StyledBackground, true);

    auto *bottomH = new QHBoxLayout(m_bottomBar);
    bottomH->setContentsMargins(24, 14, 24, 14);
    bottomH->setSpacing(10);

    m_countLabel = new QLabel("No apps selected", m_bottomBar);
    m_countLabel->setFont(ThemeManager::fontBody());

    m_installBtn = new QPushButton("Install Selected", m_bottomBar);
    m_installBtn->setFixedHeight(46);
    m_installBtn->setMinimumWidth(175);
    m_installBtn->setCursor(Qt::PointingHandCursor);
    m_installBtn->setFont(ThemeManager::fontHeading());
    m_installBtn->setEnabled(false);
    connect(m_installBtn, &QPushButton::clicked, this, &MainWindow::onInstallClicked);

    bottomH->addWidget(m_countLabel, 1);
    bottomH->addWidget(m_installBtn);
    rightV->addWidget(m_bottomBar);

    m_overlay = new ProgressOverlay(this);
    m_overlay->hide();
    connect(m_overlay, &ProgressOverlay::closed, this, [this]() { m_overlay->hide(); });

    buildGrid();
    buildPages();

    // Grid is the default view — wizard lives behind the toggle button
    m_stack->setCurrentIndex(0);
    m_wizardBtn->setText("Wizard");
    connect(m_wizardBtn, &QPushButton::clicked, this, &MainWindow::toggleWizard);

    connect(&TM(), &ThemeManager::themeChanged, this, &MainWindow::applyTheme);
    applyTheme();
}

void MainWindow::buildPages()
{
    for (const QString &cat : m_categoryOrder) {
        QVector<AppData> apps;
        for (const AppData &a : m_catalogue)
            if (a.category == cat)
                apps << a;

        auto *page = new WizardPage(cat, apps, m_stack);
        m_pages << page;
        m_stack->addWidget(page);

        connect(page, &WizardPage::backClicked, this, [this]() {
            goToPage(m_currentPage - 1);
        });
        connect(page, &WizardPage::nextClicked, this, [this]() {
            goToPage(m_currentPage + 1);
        });
        connect(page, &WizardPage::installClicked, this, &MainWindow::onInstallClicked);
        connect(page, &WizardPage::selectionChanged, this, &MainWindow::refreshInstallButton);
    }
}

void MainWindow::buildGrid()
{
    m_gridScroll = new QScrollArea(m_stack);
    m_gridScroll->setWidgetResizable(true);
    m_gridScroll->setFrameShape(QFrame::NoFrame);
    m_gridScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_gridScroll->setAttribute(Qt::WA_TranslucentBackground);

    auto *container = new QWidget();
    container->setAttribute(Qt::WA_TranslucentBackground);
    auto *grid = new QGridLayout(container);
    grid->setContentsMargins(24, 16, 24, 16);
    grid->setSpacing(12);

    for (int i = 0; i < m_catalogue.size(); ++i) {
        auto *card = new AppCard(m_catalogue[i], container);
        m_gridCards << card;
        grid->addWidget(card, i / 3, i % 3);
        connect(card, &AppCard::selectionChanged, this, &MainWindow::refreshInstallButton);
    }
    grid->setRowStretch((m_catalogue.size() / 3) + 1, 1);

    m_gridScroll->setWidget(container);
    m_stack->addWidget(m_gridScroll);
}

void MainWindow::toggleWizard()
{
    m_wizardMode = !m_wizardMode;
    if (m_wizardMode) {
        m_wizardBtn->setText("Grid");
        m_stepLabel->setVisible(true);
        goToPage(0);
    } else {
        m_stack->setCurrentIndex(0);
        m_wizardBtn->setText("Wizard");
    m_stepLabel->setVisible(m_wizardMode);
        m_titleLabel->setText("Rigset");
        m_subtitleLabel->setText("Pick the apps you want \u2014 we'll install them all at once.");
        refreshInstallButton();
    }
}

void MainWindow::filterGrid(const QString &category)
{
    for (int i = 0; i < m_catalogue.size(); ++i) {
        bool show = category.isEmpty() || m_catalogue[i].category == category;
        m_gridCards[i]->setVisible(show);
    }
}

void MainWindow::goToPage(int index)
{
    if (index < 0 || index >= m_pages.size()) return;

    m_currentPage = index;
    m_stack->setCurrentIndex(index + 1);

    WizardPage *page = m_pages[index];

    bool isFirst = (index == 0);
    bool isLast  = (index == m_pages.size() - 1);

    page->setBackVisible(!isFirst);
    page->setNextVisible(!isLast);

    m_stepLabel->setText(QStringLiteral("%1 of %2 — %3")
        .arg(index + 1).arg(m_pages.size()).arg(m_categoryOrder[index]));

    refreshInstallButton();
}

void MainWindow::refreshInstallButton()
{
    int n = 0;
    if (m_wizardMode) {
        for (WizardPage *page : m_pages)
            n += page->selectedIds().size();
    } else {
        for (AppCard *card : m_gridCards)
            if (card->isChecked()) ++n;
    }

    m_installBtn->setEnabled(n > 0);
    m_countLabel->setText(n == 0 ? "No apps selected"
                        : n == 1 ? "1 app selected"
                                 : QStringLiteral("%1 apps selected").arg(n));
    m_installBtn->setText(n > 0
        ? QStringLiteral("Install %1 App%2").arg(n).arg(n > 1 ? "s" : "")
        : "Install Selected");
}

void MainWindow::onInstallClicked()
{
    QVector<AppData> toInstall;
    if (m_wizardMode) {
        for (WizardPage *page : m_pages) {
            QVector<QString> ids = page->selectedIds();
            for (const QString &id : ids) {
                for (const AppData &app : m_catalogue) {
                    if (app.id == id) {
                        toInstall << app;
                        break;
                    }
                }
            }
        }
    } else {
        for (AppCard *card : m_gridCards) {
            if (card->isChecked())
                toInstall << card->appData();
        }
    }
    if (toInstall.isEmpty()) return;
    m_overlay->setGeometry(rect());
    m_overlay->startInstall(toInstall);
}

// ── Theme ────────────────────────────────────────────────────────────────────

void MainWindow::applyTheme()
{
    auto c = [](const QColor &col) { return col.name(QColor::HexArgb); };
    const auto &pal   = TM().colors();
    const bool  isMid = pal.hasGradientBg;

    m_centralWidget->setStyleSheet(
        isMid ? "QWidget { background: transparent; }"
              : QStringLiteral("QWidget { background-color: %1; }").arg(c(pal.bg)));

    m_gradientBg->setGeometry(m_centralWidget->rect());
    m_gradientBg->update();

    if (isMid) {
        m_headerWidget->setStyleSheet(
            "QWidget { background-color: rgba(14,10,36,215); "
            "border-bottom: 1px solid rgba(100,70,180,160); }");
    } else {
        m_headerWidget->setStyleSheet(QStringLiteral(
            "QWidget { background-color: %1; border-bottom: 1px solid %2; }")
            .arg(c(pal.glassBg)).arg(c(pal.glassBorder)));
    }

    auto setPal = [](QLabel *lbl, const QColor &col) {
        QPalette p = lbl->palette();
        p.setColor(QPalette::WindowText, col);
        lbl->setPalette(p);
    };
    setPal(m_titleLabel,    pal.textPrimary);
    setPal(m_subtitleLabel, pal.textSecondary);
    setPal(m_countLabel,    pal.textSecondary);
    setPal(m_stepLabel,     pal.textTertiary);

    {
        QString badgeBg, badgeFg, badgeBorder;
        const QString osTag = OsDetect::tag();

        if (isMid) {
            badgeBg = "rgba(80,50,160,140)";
            badgeFg = "#C8B8FF";
            badgeBorder = "rgba(130,80,255,120)";
        } else if (TM().currentTheme() == ThemeManager::Theme::Dark) {
            badgeBg = c(pal.surfaceElevated);
            badgeFg = c(pal.textSecondary);
            badgeBorder = c(pal.border);
        } else {
            badgeBg = c(pal.surfaceAlt);
            badgeFg = c(pal.textSecondary);
            badgeBorder = c(pal.border);
        }

        QString icon;
        if      (osTag == "Windows") icon = "Windows ";
        else if (osTag == "macOS")   icon = "macOS ";
        else if (osTag == "Arch")    icon = "Arch ";
        else if (osTag == "Debian")  icon = "Debian ";
        else if (osTag == "Fedora")  icon = "Fedora ";
        else                          icon = "? ";

        m_osBadge->setText(icon + OsDetect::name());
        m_osBadge->setStyleSheet(QStringLiteral(
            "QLabel { background-color: %1; color: %2; border: 1px solid %3;"
            " border-radius: 8px; padding: 2px 10px; font-size: 11px; }")
            .arg(badgeBg, badgeFg, badgeBorder));
    }

    m_searchBox->setStyleSheet(QStringLiteral(R"(
QLineEdit {
    background-color: %1; border: 1px solid %2; border-radius: 10px;
    padding: 0 12px; font-size: 13px; color: %3;
}
QLineEdit:focus { border-color: %4; background-color: %5; }
)").arg(c(pal.inputBg)).arg(c(pal.inputBorder)).arg(c(pal.textPrimary))
      .arg(c(pal.inputBorderFocus)).arg(c(pal.surface)));

    if (isMid) {
        m_bottomBar->setStyleSheet(
            "QWidget { background-color: rgba(12,8,32,220); "
            "border-top: 1px solid rgba(90,60,160,140); }");
    } else {
        m_bottomBar->setStyleSheet(QStringLiteral(
            "QWidget { background-color: %1; border-top: 1px solid %2; }")
            .arg(c(pal.glassBg)).arg(c(pal.glassBorder)));
    }

    if (isMid) {
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
QPushButton:pressed {
    background: qlineargradient(x1:0,y1:0,x2:1,y2:1,
        stop:0 #7040E0, stop:1 #4820C0);
}
QPushButton:disabled {
    background: rgba(60,50,100,180); color: rgba(150,140,200,130);
}
)"));
    } else {
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

    m_stepLabel->setVisible(false);

    if (isMid) {
        m_wizardBtn->setStyleSheet(QStringLiteral(R"(
QPushButton {
    background: transparent; color: %1; border: 1px solid %2;
    border-radius: 8px; padding: 4px 14px; font-size: 12px;
}
QPushButton:hover { background: rgba(138,80,255,40); color: %3; }
)").arg(c(pal.textSecondary)).arg(c(QColor(130,80,255,120))).arg(c(pal.textPrimary)));
    } else {
        m_wizardBtn->setStyleSheet(QStringLiteral(R"(
QPushButton {
    background: transparent; color: %1; border: 1px solid %2;
    border-radius: 8px; padding: 4px 14px; font-size: 12px;
}
QPushButton:hover { background: %3; color: %4; }
)").arg(c(pal.textSecondary)).arg(c(pal.border)).arg(c(pal.accentSubtle)).arg(c(pal.textPrimary)));
    }

    for (WizardPage *page : m_pages)
        page->applyTheme();
}

void MainWindow::resizeEvent(QResizeEvent *e)
{
    QMainWindow::resizeEvent(e);
    if (m_gradientBg) m_gradientBg->setGeometry(m_centralWidget->rect());
    if (m_overlay) m_overlay->setGeometry(rect());
}

void MainWindow::applyFilter() {}
