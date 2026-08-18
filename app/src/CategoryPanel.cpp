#include "CategoryPanel.h"
#include "Theme.h"

#include <QPushButton>
#include <QLabel>
#include <QFrame>
#include <QPainter>
#include <QPainterPath>
#include <QLinearGradient>

CategoryPanel::CategoryPanel(const QStringList &categories, QWidget *parent)
    : QWidget(parent)
{
    setFixedWidth(178);

    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(10, 28, 10, 24);
    m_layout->setSpacing(2);

    m_group = new QButtonGroup(this);
    m_group->setExclusive(true);

    m_header = new QLabel("Categories", this);
    m_header->setFont(ThemeManager::fontCaption());
    m_header->setContentsMargins(8, 0, 0, 10);
    m_layout->addWidget(m_header);

    addRow("All Apps", "");

    auto *div = new QFrame(this);
    div->setFrameShape(QFrame::HLine);
    div->setFixedHeight(1);
    m_layout->addSpacing(6);
    m_layout->addWidget(div);
    m_layout->addSpacing(6);

    for (const QString &cat : categories)
        addRow(cat, cat);

    m_layout->addStretch();

    if (!m_rows.isEmpty())
        m_rows.first().btn->setChecked(true);

    connect(&TM(), &ThemeManager::themeChanged, this, &CategoryPanel::applyTheme);
    applyTheme();
}

// ── paint ─────────────────────────────────────────────────────────────────────

void CategoryPanel::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const auto &pal  = TM().colors();
    const bool  isMid = pal.hasGradientBg;
    const QRect  r   = rect();

    if (isMid) {
        // Deep gradient sidebar: dark indigo top → slightly bluer bottom
        QLinearGradient grad(0, 0, 0, r.height());
        grad.setColorAt(0.0, QColor( 18,  12,  46));
        grad.setColorAt(0.5, QColor( 14,  10,  38));
        grad.setColorAt(1.0, QColor( 12,   8,  32));
        p.setBrush(grad);
        p.setPen(Qt::NoPen);
        p.drawRect(r);

        // Subtle inner-left highlight (depth illusion)
        QLinearGradient hilight(0, 0, 18, 0);
        hilight.setColorAt(0, QColor(255, 255, 255, 8));
        hilight.setColorAt(1, Qt::transparent);
        p.setBrush(hilight);
        p.drawRect(r);

        // Right border: gradient violet glow
        QLinearGradient borderGrad(r.topRight(), r.bottomRight());
        borderGrad.setColorAt(0.0, QColor(130,  80, 255, 120));
        borderGrad.setColorAt(0.5, QColor( 90,  60, 200, 160));
        borderGrad.setColorAt(1.0, QColor( 60,  40, 160,  80));
        p.setBrush(Qt::NoBrush);
        p.setPen(QPen(QBrush(borderGrad), 1.0));
        p.drawLine(r.topRight() + QPoint(-1, 0), r.bottomRight() + QPoint(-1, 0));

        // Shadow cast to the right
        QLinearGradient shadow(r.right() - 1, 0, r.right() + 14, 0);
        shadow.setColorAt(0, QColor(0, 0, 20, 90));
        shadow.setColorAt(1, Qt::transparent);
        p.setBrush(shadow);
        p.setPen(Qt::NoPen);
        p.drawRect(r.right() - 1, 0, 14, r.height());

    } else {
        // Light / Dark: simple glass fill + right border
        p.setBrush(pal.glassBg);
        p.setPen(Qt::NoPen);
        p.drawRect(r);

        p.setBrush(Qt::NoBrush);
        p.setPen(QPen(pal.glassBorder, 1));
        p.drawLine(r.topRight() + QPoint(-1, 0), r.bottomRight() + QPoint(-1, 0));
    }
}

// ── applyTheme ────────────────────────────────────────────────────────────────

void CategoryPanel::applyTheme()
{
    QPalette hp = m_header->palette();
    hp.setColor(QPalette::WindowText, TM().textTertiary());
    m_header->setPalette(hp);

    for (int i = 0; i < m_layout->count(); ++i) {
        auto *w = m_layout->itemAt(i)->widget();
        if (auto *fr = qobject_cast<QFrame*>(w)) {
            QPalette fp = fr->palette();
            fp.setColor(QPalette::Mid, TM().colors().borderSubtle);
            fr->setPalette(fp);
        }
    }

    const QString ss = buttonStyleSheet();
    for (const Row &row : m_rows)
        row.btn->setStyleSheet(ss);

    update();
}

// ── private ───────────────────────────────────────────────────────────────────

QString CategoryPanel::buttonStyleSheet() const
{
    auto c = [](const QColor &col) { return col.name(QColor::HexArgb); };
    const auto &pal  = TM().colors();
    const bool  isMid = pal.hasGradientBg;

    if (isMid) {
        // Midnight: active = gradient pill with glow effect via background
        return QStringLiteral(R"(
QPushButton {
    text-align: left;
    padding: 8px 14px;
    border: none;
    border-radius: 10px;
    background: transparent;
    color: %1;
    font-size: 13px;
}
QPushButton:hover {
    background-color: %2;
    color: %3;
}
QPushButton:checked {
    background-color: %4;
    color: #FFFFFF;
    font-weight: 600;
    border: 1px solid %5;
}
)")
        .arg(c(pal.textSecondary))
        .arg(c(QColor(138, 80, 255, 28)))
        .arg(c(pal.textPrimary))
        .arg(c(QColor(138, 80, 255, 55)))
        .arg(c(QColor(160, 100, 255, 160)));
    }

    return QStringLiteral(R"(
QPushButton {
    text-align: left;
    padding: 8px 14px;
    border: none;
    border-radius: 10px;
    background: transparent;
    color: %1;
    font-size: 13px;
}
QPushButton:hover {
    background-color: %2;
}
QPushButton:checked {
    background-color: %3;
    color: %4;
    font-weight: 600;
}
)")
    .arg(c(pal.textPrimary))
    .arg(c(pal.accentSubtle))
    .arg(c(pal.accent))
    .arg(c(pal.accentText));
}

void CategoryPanel::addRow(const QString &label, const QString &value)
{
    auto *btn = new QPushButton(label, this);
    btn->setCheckable(true);
    btn->setFont(ThemeManager::fontBody());
    btn->setCursor(Qt::PointingHandCursor);
    btn->setProperty("catValue", value);

    m_group->addButton(btn, m_rows.size());
    m_layout->addWidget(btn);
    m_rows.append({ btn, value });

    connect(btn, &QPushButton::clicked, this, [this, value]() {
        emit categorySelected(value);
    });
}
