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
// GradientBackground — transparent QWidget that paints the Midnight gradient
// behind everything else in the central widget
// ═════════════════════════════════════════════════════════════════════════════

class GradientBackground : public QWidget
{
    Q_OBJECT
public:
    explicit GradientBackground(QWidget *parent = nullptr) : QWidget(parent)
    {
        setAttribute(Qt::WA_TransparentForMouseEvents);
        setAttribute(Qt::WA_NoSystemBackground);
        lower();  // always behind everything
        connect(&TM(), &ThemeManager::themeChanged, this, [this]{ update(); });
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        const auto &pal = TM().colors();
        if (!pal.hasGradientBg) {
            // Just fill with bg color for Light/Dark
            QPainter p(this);
            p.fillRect(rect(), pal.bg);
            return;
        }

        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);

        // Main diagonal gradient
        QLinearGradient grad(0, 0, width(), height());
        grad.setColorAt(0.0, pal.gradA);
        grad.setColorAt(0.45, pal.gradB);
        grad.setColorAt(1.0, pal.gradC);
        p.fillRect(rect(), grad);

        // Radial nebula glow — top-right corner (purple haze)
        QRadialGradient nebulaA(width() * 0.82, height() * 0.12, width() * 0.45);
        nebulaA.setColorAt(0.0, QColor(110, 50, 200, 55));
        nebulaA.setColorAt(1.0, Qt::transparent);
        p.fillRect(rect(), nebulaA);

        // Radial nebula glow — bottom-left corner (blue haze)
        QRadialGradient nebulaB(width() * 0.15, height() * 0.88, width() * 0.4);
        nebulaB.setColorAt(0.0, QColor(40, 60, 160, 45));
        nebulaB.setColorAt(1.0, Qt::transparent);
        p.fillRect(rect(), nebulaB);

        // Faint star-like dots (deterministic positions using width/height as seed)
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

#include "MainWindow.moc"   // needed because GradientBackground is Q_OBJECT in .cpp

// ═════════════════════════════════════════════════════════════════════════════
// ThemePicker — redesigned with color dot swatches
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
        { ThemeManager::Theme::Light,    "Light",    { 255, 210,  60 } },
        { ThemeManager::Theme::Dark,     "Dark",     {  70, 140, 255 } },
        { ThemeManager::Theme::Midnight, "Midnight", { 150,  80, 255 } },
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

    // Pill container background
    if (isMid) {
        setStyleSheet(QStringLiteral(
            "QWidget { background-color: rgba(30,20,60,200); border-radius: 12px; }"));
    } else {
        setStyleSheet(QStringLiteral(
            "QWidget { background-color: %1; border-radius: 12px; }")
            .arg(c(pal.surfaceAlt)));
    }

    for (const auto &sw : m_swatches) {
        bool active = (sw.themeId == TM().currentTheme());
        QColor dotColor = sw.dot;

        if (isMid && active) {
            sw.btn->setStyleSheet(QStringLiteral(R"(
QPushButton {
    background: qlineargradient(x1:0,y1:0,x2:1,y2:1,
        stop:0 rgba(120,60,220,200), stop:1 rgba(80,40,180,200));
    color: #FFFFFF;
    border: 1px solid rgba(180,100,255,150);
    border-radius: 9px;
    padding: 0 10px;
    font-size: 11px;
    font-weight: 600;
}
QPushButton:hover {
    background: qlineargradient(x1:0,y1:0,x2:1,y2:1,
        stop:0 rgba(150,80,255,220), stop:1 rgba(100,60,200,220));
}
)"));
        } else if (active) {
            sw.btn->setStyleSheet(QStringLiteral(R"(
QPushButton {
    background: %1;
    color: %2;
    border: none;
    border-radius: 9px;
    padding: 0 10px;
    font-size: 11px;
    font-weight: 600;
}
QPushButton:hover { background: %3; }
)")
            .arg(c(pal.surface))
            .arg(c(pal.textPrimary))
            .arg(c(pal.surfaceElevated)));
        } else {
            sw.btn->setStyleSheet(QStringLiteral(R"(
QPushButton {
    background: transparent;
    color: %1;
    border: none;
    border-radius: 9px;
    padding: 0 10px;
    font-size: 11px;
    font-weight: 400;
}
QPushButton:hover { background-color: %2; }
)")
            .arg(c(pal.textSecondary))
            .arg(c(pal.accentSubtle)));
        }
    }
    update();
}

void ThemePicker::paintEvent(QPaintEvent *)
{
    // Draw colored dot beside each button label
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    for (const auto &sw : m_swatches) {
        QRect br = sw.btn->geometry();
        // Dot sits left of text, vertically centered
        int dotX = br.left() + 10;
        int dotY = br.center().y();
        int dotR = 4;

        QColor col = sw.dot;
        bool active = (sw.themeId == TM().currentTheme());

        if (!active) col.setAlpha(140);

        p.setPen(Qt::NoPen);
        p.setBrush(col);
        p.drawEllipse(dotX, dotY - dotR, dotR * 2, dotR * 2);

        if (active) {
            // Glow
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
    setWindowTitle("App Installer");
    setMinimumSize(960, 660);
    resize(1180, 760);

    m_catalogue = defaultCatalogue();

    QStringList categories;
    for (const AppData &a : m_catalogue)
        if (!categories.contains(a.category))
            categories << a.category;

    // ── Central widget ────────────────────────────────────────────────────────
    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);

    // Gradient bg layer (sits behind everything)
    m_gradientBg = new GradientBackground(m_centralWidget);
    m_gradientBg->setGeometry(m_centralWidget->rect());

    auto *rootH = new QHBoxLayout(m_centralWidget);
    rootH->setContentsMargins(0, 0, 0, 0);
    rootH->setSpacing(0);

    // ── Sidebar ───────────────────────────────────────────────────────────────
    m_sidebar = new CategoryPanel(categories, m_centralWidget);
    connect(m_sidebar, &CategoryPanel::categorySelected,
            this, &MainWindow::onCategorySelected);
    rootH->addWidget(m_sidebar);

    // ── Right pane ────────────────────────────────────────────────────────────
    auto *rightPane = new QWidget(m_centralWidget);
    rightPane->setAttribute(Qt::WA_TranslucentBackground);
    auto *rightV = new QVBoxLayout(rightPane);
    rightV->setContentsMargins(0, 0, 0, 0);
    rightV->setSpacing(0);
    rootH->addWidget(rightPane, 1);

    buildHeader(rightPane, rightV);

    // Scroll area
    m_scrollArea = new QScrollArea(rightPane);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setAttribute(Qt::WA_TranslucentBackground);
    m_scrollArea->viewport()->setAttribute(Qt::WA_TranslucentBackground);
    rightV->addWidget(m_scrollArea, 1);

    m_gridContainer = new QWidget;
    m_gridContainer->setAttribute(Qt::WA_TranslucentBackground);
    m_grid = new QGridLayout(m_gridContainer);
    m_grid->setContentsMargins(24, 16, 24, 24);
    m_grid->setSpacing(ThemeManager::cardSpacing);
    m_scrollArea->setWidget(m_gridContainer);

    // ── Bottom bar ────────────────────────────────────────────────────────────
    m_bottomBar = new QWidget(rightPane);
    m_bottomBar->setFixedHeight(72);
    m_bottomBar->setAttribute(Qt::WA_StyledBackground, true);

    auto *bottomH = new QHBoxLayout(m_bottomBar);
    bottomH->setContentsMargins(24, 14, 24, 14);
    bottomH->setSpacing(10);

    m_countLabel = new QLabel("No apps selected", m_bottomBar);
    m_countLabel->setFont(ThemeManager::fontBody());

    m_selectAllBtn = new QPushButton("Select All", m_bottomBar);
    m_selectAllBtn->setFixedHeight(34);
    m_selectAllBtn->setCursor(Qt::PointingHandCursor);
    m_selectAllBtn->setFont(ThemeManager::fontBody());
    connect(m_selectAllBtn, &QPushButton::clicked, this, &MainWindow::onSelectAll);

    m_deselectBtn = new QPushButton("Deselect All", m_bottomBar);
    m_deselectBtn->setFixedHeight(34);
    m_deselectBtn->setCursor(Qt::PointingHandCursor);
    m_deselectBtn->setFont(ThemeManager::fontBody());
    connect(m_deselectBtn, &QPushButton::clicked, this, &MainWindow::onDeselectAll);

    m_installBtn = new QPushButton("Install Selected", m_bottomBar);
    m_installBtn->setFixedHeight(46);
    m_installBtn->setMinimumWidth(175);
    m_installBtn->setCursor(Qt::PointingHandCursor);
    m_installBtn->setFont(ThemeManager::fontHeading());
    m_installBtn->setEnabled(false);
    connect(m_installBtn, &QPushButton::clicked, this, &MainWindow::onInstallClicked);

    bottomH->addWidget(m_countLabel, 1);
    bottomH->addWidget(m_selectAllBtn);
    bottomH->addWidget(m_deselectBtn);
    bottomH->addWidget(m_installBtn);
    rightV->addWidget(m_bottomBar);

    // ── Overlay ───────────────────────────────────────────────────────────────
    m_overlay = new ProgressOverlay(this);
    m_overlay->hide();
    connect(m_overlay, &ProgressOverlay::closed, this, [this]() { m_overlay->hide(); });

    // ── Cards ─────────────────────────────────────────────────────────────────
    buildGrid();
    applyFilter();

    connect(&TM(), &ThemeManager::themeChanged, this, &MainWindow::applyTheme);
    applyTheme();
}

// ── buildHeader ───────────────────────────────────────────────────────────────

void MainWindow::buildHeader(QWidget *parent, QVBoxLayout *layout)
{
    m_headerWidget = new QWidget(parent);
    m_headerWidget->setFixedHeight(126);
    m_headerWidget->setAttribute(Qt::WA_StyledBackground, true);

    auto *hLayout = new QVBoxLayout(m_headerWidget);
    hLayout->setContentsMargins(28, 20, 28, 14);
    hLayout->setSpacing(6);

    m_titleLabel = new QLabel("App Installer", m_headerWidget);
    m_titleLabel->setFont(ThemeManager::fontTitle());

    m_subtitleLabel = new QLabel(
        "Pick the apps you want — we'll install them all at once.",
        m_headerWidget);
    m_subtitleLabel->setFont(ThemeManager::fontBody());

    m_searchBox = new QLineEdit(m_headerWidget);
    m_searchBox->setPlaceholderText("Search apps…");
    m_searchBox->setFixedHeight(32);
    m_searchBox->setMaximumWidth(240);
    connect(m_searchBox, &QLineEdit::textChanged, this, &MainWindow::onSearchChanged);

    m_themePicker = new ThemePicker(m_headerWidget);

    // OS badge — shows the detected OS name
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
    topRow->addWidget(m_themePicker);
    topRow->addWidget(m_searchBox);

    hLayout->addLayout(topRow);
    hLayout->addWidget(m_subtitleLabel);

    layout->addWidget(m_headerWidget);
}

// ── applyTheme ────────────────────────────────────────────────────────────────

void MainWindow::applyTheme()
{
    auto c = [](const QColor &col) { return col.name(QColor::HexArgb); };
    const auto &pal   = TM().colors();
    const bool  isMid = pal.hasGradientBg;

    // Central widget: transparent so gradient bg shows through
    m_centralWidget->setStyleSheet(
        isMid ? "QWidget { background: transparent; }"
              : QStringLiteral("QWidget { background-color: %1; }").arg(c(pal.bg)));

    // Size and reposition gradient bg layer
    m_gradientBg->setGeometry(m_centralWidget->rect());
    m_gradientBg->update();

    // Header
    if (isMid) {
        m_headerWidget->setStyleSheet(QStringLiteral(
            "QWidget { background-color: rgba(14,10,36,215); "
            "border-bottom: 1px solid rgba(100,70,180,160); }"));
    } else {
        m_headerWidget->setStyleSheet(QStringLiteral(
            "QWidget { background-color: %1; border-bottom: 1px solid %2; }")
            .arg(c(pal.glassBg)).arg(c(pal.glassBorder)));
    }

    // Labels
    auto setPal = [](QLabel *lbl, const QColor &col) {
        QPalette p = lbl->palette();
        p.setColor(QPalette::WindowText, col);
        lbl->setPalette(p);
    };
    setPal(m_titleLabel,    pal.textPrimary);
    setPal(m_subtitleLabel, pal.textSecondary);
    setPal(m_countLabel,    pal.textSecondary);

    // OS badge pill
    {
        auto c2 = [](const QColor &col) { return col.name(QColor::HexArgb); };
        QString badgeBg, badgeFg, badgeBorder;
        const QString osTag = OsDetect::tag();

        if (isMid) {
            badgeBg     = "rgba(80,50,160,140)";
            badgeFg     = "#C8B8FF";
            badgeBorder = "rgba(130,80,255,120)";
        } else if (TM().currentTheme() == ThemeManager::Theme::Dark) {
            badgeBg     = c2(pal.surfaceElevated);
            badgeFg     = c2(pal.textSecondary);
            badgeBorder = c2(pal.border);
        } else {
            badgeBg     = c2(pal.surfaceAlt);
            badgeFg     = c2(pal.textSecondary);
            badgeBorder = c2(pal.border);
        }

        // Pick a small icon per OS
        QString icon;
        if      (osTag == "Windows") icon = "⊞ ";
        else if (osTag == "macOS")   icon = " ";
        else if (osTag == "Arch")    icon = "󰣇 ";
        else if (osTag == "Debian")  icon = " ";
        else if (osTag == "Fedora")  icon = " ";
        else                          icon = "? ";

        m_osBadge->setText(icon + OsDetect::name());
        m_osBadge->setStyleSheet(QStringLiteral(
            "QLabel { background-color: %1; color: %2; border: 1px solid %3;"
            " border-radius: 8px; padding: 2px 10px; font-size: 11px; }")
            .arg(badgeBg, badgeFg, badgeBorder));
    }

    // Search box
    m_searchBox->setStyleSheet(QStringLiteral(R"(
QLineEdit {
    background-color: %1;
    border: 1px solid %2;
    border-radius: 10px;
    padding: 0 12px;
    font-size: 13px;
    color: %3;
}
QLineEdit:focus {
    border-color: %4;
    background-color: %5;
}
)")
    .arg(c(pal.inputBg))
    .arg(c(pal.inputBorder))
    .arg(c(pal.textPrimary))
    .arg(c(pal.inputBorderFocus))
    .arg(c(pal.surface)));

    // Bottom bar
    if (isMid) {
        m_bottomBar->setStyleSheet(QStringLiteral(
            "QWidget { background-color: rgba(12,8,32,220); "
            "border-top: 1px solid rgba(90,60,160,140); }"));
    } else {
        m_bottomBar->setStyleSheet(QStringLiteral(
            "QWidget { background-color: %1; border-top: 1px solid %2; }")
            .arg(c(pal.glassBg)).arg(c(pal.glassBorder)));
    }

    // Select All (outlined accent)
    if (isMid) {
        m_selectAllBtn->setStyleSheet(QStringLiteral(R"(
QPushButton {
    background: transparent;
    border: 1px solid rgba(138,80,255,180);
    border-radius: 10px;
    color: rgba(180,140,255,255);
    padding: 0 14px;
    font-size: 13px;
}
QPushButton:hover { background-color: rgba(138,80,255,30); }
QPushButton:pressed { background-color: rgba(138,80,255,55); }
)"));
    } else {
        m_selectAllBtn->setStyleSheet(QStringLiteral(R"(
QPushButton {
    background: transparent;
    border: 1px solid %1;
    border-radius: 10px;
    color: %1;
    padding: 0 14px;
    font-size: 13px;
}
QPushButton:hover   { background-color: %2; }
QPushButton:pressed { background-color: %3; }
)")
        .arg(c(pal.accent))
        .arg(c(pal.accentSubtle))
        .arg(c(pal.cardSelected)));
    }

    // Deselect (muted)
    m_deselectBtn->setStyleSheet(QStringLiteral(R"(
QPushButton {
    background: transparent;
    border: 1px solid %1;
    border-radius: 10px;
    color: %2;
    padding: 0 14px;
    font-size: 13px;
}
QPushButton:hover   { background-color: %3; }
QPushButton:pressed { background-color: %4; }
)")
    .arg(c(pal.border))
    .arg(c(pal.textSecondary))
    .arg(c(pal.surfaceAlt))
    .arg(c(pal.surfaceElevated)));

    // Install button
    if (isMid) {
        m_installBtn->setStyleSheet(QStringLiteral(R"(
QPushButton {
    background: qlineargradient(x1:0,y1:0,x2:1,y2:1,
        stop:0 #8A50FF, stop:0.5 #6A3AEF, stop:1 #5028D0);
    color: #FFFFFF;
    border: none;
    border-radius: 14px;
    font-size: 14px;
    font-weight: 700;
    padding: 0 28px;
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
    background: rgba(60,50,100,180);
    color: rgba(150,140,200,130);
}
)"));
    } else {
        m_installBtn->setStyleSheet(QStringLiteral(R"(
QPushButton {
    background-color: %1;
    color: %2;
    border: none;
    border-radius: 14px;
    font-size: 14px;
    font-weight: 700;
    padding: 0 28px;
}
QPushButton:hover   { background-color: %3; }
QPushButton:pressed { background-color: %4; }
QPushButton:disabled { background-color: %5; color: %6; }
)")
        .arg(c(pal.accent))
        .arg(c(pal.accentText))
        .arg(c(pal.accentHover))
        .arg(c(pal.accentPressed))
        .arg(c(pal.accentDisabled))
        .arg(c(pal.textDisabled)));
    }

    // Grid container: transparent so gradient bg shows through
    m_gridContainer->setStyleSheet("background: transparent;");
}

// ── grid ──────────────────────────────────────────────────────────────────────

void MainWindow::buildGrid()
{
    for (const AppData &app : m_catalogue) {
        auto *card = new AppCard(app, m_gridContainer);
        card->setAvailable(app.availableOnCurrentOs());
        m_cards[app.id] = card;
        connect(card, &AppCard::selectionChanged,
                this, &MainWindow::onCardSelectionChanged);
    }
}

void MainWindow::applyFilter()
{
    while (m_grid->count()) {
        auto *item = m_grid->takeAt(0);
        if (item->widget()) item->widget()->hide();
        delete item;
    }

    QVector<AppCard*> visible;
    for (const AppData &app : m_catalogue) {
        bool catOk  = m_activeCategory.isEmpty() || app.category == m_activeCategory;
        bool srchOk = m_searchText.isEmpty()
                   || app.name.contains(m_searchText, Qt::CaseInsensitive)
                   || app.description.contains(m_searchText, Qt::CaseInsensitive);
        if (catOk && srchOk) visible << m_cards[app.id];
    }

    int availW = m_scrollArea->viewport()->width() - 48;
    int cols   = qMax(1, availW / (218 + ThemeManager::cardSpacing));

    int row = 0, col = 0;
    for (AppCard *card : visible) {
        card->show();
        m_grid->addWidget(card, row, col);
        if (++col >= cols) { col = 0; ++row; }
    }
    m_grid->setRowStretch(row + 1, 1);
    m_grid->setColumnStretch(cols, 1);
}

// ── events ────────────────────────────────────────────────────────────────────

void MainWindow::resizeEvent(QResizeEvent *e)
{
    QMainWindow::resizeEvent(e);
    if (m_gradientBg) m_gradientBg->setGeometry(m_centralWidget->rect());
    applyFilter();
    if (m_overlay) m_overlay->setGeometry(rect());
}

// ── slots ─────────────────────────────────────────────────────────────────────

void MainWindow::onCategorySelected(const QString &category)
{ m_activeCategory = category; applyFilter(); }

void MainWindow::onSearchChanged(const QString &text)
{ m_searchText = text; applyFilter(); }

void MainWindow::onCardSelectionChanged(const QString &id, bool selected)
{
    m_selectedIds.removeAll(id);
    if (selected) m_selectedIds << id;
    refreshInstallButton();
}

void MainWindow::onInstallClicked()
{
    QVector<AppData> toInstall;
    for (const AppData &app : m_catalogue)
        if (m_selectedIds.contains(app.id))
            toInstall << app;
    if (toInstall.isEmpty()) return;
    m_overlay->setGeometry(rect());
    m_overlay->startInstall(toInstall);
}

void MainWindow::onSelectAll()
{
    for (auto it = m_cards.begin(); it != m_cards.end(); ++it)
        if (!it.value()->isHidden() && it.value()->isAvailable())
            it.value()->setChecked(true);
}

void MainWindow::onDeselectAll()
{
    for (auto it = m_cards.begin(); it != m_cards.end(); ++it)
        it.value()->setChecked(false);
}

void MainWindow::refreshInstallButton()
{
    int n = m_selectedIds.size();
    m_installBtn->setEnabled(n > 0);
    m_countLabel->setText(n == 0 ? "No apps selected"
                        : n == 1 ? "1 app selected"
                                 : QStringLiteral("%1 apps selected").arg(n));
    m_installBtn->setText(n > 0
        ? QStringLiteral("Install %1 App%2").arg(n).arg(n > 1 ? "s" : "")
        : "Install Selected");
}
