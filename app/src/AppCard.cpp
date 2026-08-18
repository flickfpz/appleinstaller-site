#include "AppCard.h"
#include "Theme.h"
#include "OsDetect.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QRadialGradient>
#include <QLinearGradient>

// ── helpers ───────────────────────────────────────────────────────────────────

static void drawMultiShadow(QPainter &p, const QPainterPath &path,
                             const QColor &base, qreal intensity)
{
    // Three shadow passes: wide soft, medium, tight crisp
    struct Pass { qreal dy; qreal blur; int alpha; };
    const Pass passes[] = {
        { 6,  20, int(30 * intensity) },
        { 3,   8, int(45 * intensity) },
        { 1.5, 3, int(30 * intensity) },
    };
    for (auto &pass : passes) {
        QColor sc = base;
        sc.setAlpha(pass.alpha);
        // Approximate blur by drawing at multiple offsets with low alpha
        const int steps = 5;
        for (int i = 0; i < steps; ++i) {
            qreal t    = qreal(i) / steps;
            qreal doff = pass.dy * t;
            QColor sc2 = sc;
            sc2.setAlpha(int(pass.alpha * (1.0 - t * 0.5) / steps));
            p.setBrush(sc2);
            p.drawPath(path.translated(0, pass.dy - doff * 0.3));
        }
    }
}

// ── Icon painter ──────────────────────────────────────────────────────────────

QPixmap AppCard::makeIconPixmap(const QString &iconName, int size)
{
    if (!iconName.isEmpty()) {
        QPixmap pm(iconName);
        if (!pm.isNull())
            return pm.scaled(size, size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    QPixmap pm(size, size);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);

    const QColor a = TM().accent();
    const bool midnight = TM().colors().hasGradientBg;

    QPainterPath path;
    path.addRoundedRect(0, 0, size, size, size * 0.24, size * 0.24);

    if (midnight) {
        // Neon gradient for midnight: purple → blue → cyan shimmer
        QLinearGradient g(0, 0, size, size);
        g.setColorAt(0.0, QColor(160,  70, 255));
        g.setColorAt(0.5, QColor( 90,  80, 255));
        g.setColorAt(1.0, QColor( 60, 180, 255));
        p.fillPath(path, g);

        // Inner glow
        QRadialGradient glow(size * 0.35, size * 0.25, size * 0.7);
        glow.setColorAt(0, QColor(255, 255, 255, 55));
        glow.setColorAt(1, Qt::transparent);
        p.fillPath(path, glow);
    } else {
        QLinearGradient g(0, 0, size, size);
        g.setColorAt(0, a.lighter(120));
        g.setColorAt(1, a.darker(140));
        p.fillPath(path, g);
        QRadialGradient glow(size * 0.4, size * 0.3, size * 0.6);
        glow.setColorAt(0, QColor(255, 255, 255, 40));
        glow.setColorAt(1, Qt::transparent);
        p.fillPath(path, glow);
    }

    p.setPen(Qt::white);
    QFont f = ThemeManager::fontHeading();
    f.setPixelSize(size / 2);
    f.setWeight(QFont::Bold);
    p.setFont(f);
    p.drawText(QRect(0, 0, size, size), Qt::AlignCenter,
               iconName.isEmpty() ? "?" : iconName.left(1).toUpper());
    return pm;
}

// ── Constructor ───────────────────────────────────────────────────────────────

AppCard::AppCard(const AppData &data, QWidget *parent)
    : QWidget(parent), m_data(data)
{
    setFixedSize(218, 104);
    setCursor(Qt::PointingHandCursor);
    setAttribute(Qt::WA_Hover);

    auto *root = new QHBoxLayout(this);
    root->setContentsMargins(16, 16, 16, 16);
    root->setSpacing(12);

    m_iconLabel = new QLabel(this);
    m_iconLabel->setFixedSize(46, 46);
    m_iconLabel->setPixmap(makeIconPixmap(data.name, 46));
    m_iconLabel->setScaledContents(true);
    root->addWidget(m_iconLabel, 0, Qt::AlignTop);

    auto *col = new QVBoxLayout;
    col->setSpacing(3);
    col->setContentsMargins(0, 0, 0, 0);

    m_nameLabel = new QLabel(data.name, this);
    m_nameLabel->setFont(ThemeManager::fontHeading());

    m_descLabel = new QLabel(data.description, this);
    m_descLabel->setFont(ThemeManager::fontCaption());
    m_descLabel->setWordWrap(true);

    m_verLabel = new QLabel(data.version, this);
    m_verLabel->setFont(ThemeManager::fontCaption());

    col->addWidget(m_nameLabel);
    col->addWidget(m_descLabel);
    col->addStretch();
    col->addWidget(m_verLabel);
    root->addLayout(col, 1);

    m_hoverAnim = new QPropertyAnimation(this, "hoverOpacity", this);
    m_hoverAnim->setDuration(200);
    m_hoverAnim->setEasingCurve(QEasingCurve::OutCubic);

    m_pressAnim = new QPropertyAnimation(this, "pressScale", this);
    m_pressAnim->setDuration(100);
    m_pressAnim->setEasingCurve(QEasingCurve::OutQuad);

    connect(&TM(), &ThemeManager::themeChanged, this, &AppCard::applyTheme);
    applyTheme();
}

// ── Theme ─────────────────────────────────────────────────────────────────────

void AppCard::applyTheme()
{
    // Refresh icon for new accent color
    m_iconLabel->setPixmap(makeIconPixmap(m_data.name, 46));

    auto setPal = [](QLabel *lbl, const QColor &col) {
        QPalette p = lbl->palette();
        p.setColor(QPalette::WindowText, col);
        lbl->setPalette(p);
    };
    setPal(m_nameLabel, TM().textPrimary());
    setPal(m_descLabel, TM().textSecondary());
    setPal(m_verLabel,  TM().textTertiary());
    update();
}

// ── State ─────────────────────────────────────────────────────────────────────

void AppCard::setChecked(bool checked)
{
    if (!m_available) return;   // can't select unavailable apps
    if (m_checked == checked) return;
    m_checked = checked;
    update();
    emit selectionChanged(m_data.id, m_checked);
}

void AppCard::setAvailable(bool available)
{
    m_available = available;
    setEnabled(available);
    setToolTip(available ? QString()
        : QStringLiteral("Not available on %1").arg(OsDetect::tag()));
    update();
}

void AppCard::toggle() { setChecked(!m_checked); }

// ── Events ────────────────────────────────────────────────────────────────────

void AppCard::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        m_pressAnim->stop();
        m_pressAnim->setStartValue(m_pressScale);
        m_pressAnim->setEndValue(0.95);
        m_pressAnim->start();
    }
    QWidget::mousePressEvent(e);
}

void AppCard::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        m_pressAnim->stop();
        m_pressAnim->setStartValue(m_pressScale);
        m_pressAnim->setEndValue(1.0);
        m_pressAnim->start();
        if (rect().contains(e->pos())) toggle();
    }
    QWidget::mouseReleaseEvent(e);
}

void AppCard::enterEvent(QEnterEvent *e)
{
    m_hoverAnim->stop();
    m_hoverAnim->setStartValue(m_hoverOpacity);
    m_hoverAnim->setEndValue(1.0);
    m_hoverAnim->start();
    QWidget::enterEvent(e);
}

void AppCard::leaveEvent(QEvent *e)
{
    m_hoverAnim->stop();
    m_hoverAnim->setStartValue(m_hoverOpacity);
    m_hoverAnim->setEndValue(0.0);
    m_hoverAnim->start();
    QWidget::leaveEvent(e);
}

// ── Paint ─────────────────────────────────────────────────────────────────────

void AppCard::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);

    // Press-scale transform
    if (!qFuzzyCompare(m_pressScale, 1.0)) {
        QPointF center = QRectF(rect()).center();
        p.translate(center);
        p.scale(m_pressScale, m_pressScale);
        p.translate(-center);
    }

    const QRectF  r       = QRectF(rect()).adjusted(4, 4, -4, -4);
    const qreal   radius  = 16.0;                  // larger, rounder corners
    const auto   &pal     = TM().colors();
    const bool    isMid   = pal.hasGradientBg;
    const qreal   hov     = m_hoverOpacity;

    QPainterPath path;
    path.addRoundedRect(r, radius, radius);

    // ── Multi-layer shadow ────────────────────────────────────────────────────
    p.setPen(Qt::NoPen);
    {
        qreal shadowIntensity = m_checked ? 1.0 : (0.6 + 0.4 * hov);
        if (isMid) shadowIntensity *= 1.8;   // darker bg = stronger shadow needed

        // Layer 1: wide diffuse
        QColor s1 = pal.shadowColor;
        s1.setAlpha(int(s1.alpha() * shadowIntensity * 0.6));
        p.setBrush(s1);
        p.drawPath(path.translated(0, 8).translated(0, 0));

        // Layer 2: medium
        QColor s2 = pal.shadowColor;
        s2.setAlpha(int(s2.alpha() * shadowIntensity * 0.9));
        p.setBrush(s2);
        p.drawPath(path.translated(0, 4));

        // Layer 3: tight
        QColor s3 = pal.shadowColor;
        s3.setAlpha(int(s3.alpha() * shadowIntensity));
        p.setBrush(s3);
        p.drawPath(path.translated(0, 2));
    }

    // ── Card background ───────────────────────────────────────────────────────
    if (isMid) {
        // Gradient card: dark indigo → slightly lighter toward bottom-right
        QLinearGradient cardGrad(r.topLeft(), r.bottomRight());
        if (m_checked) {
            // Selected: tinted with purple
            cardGrad.setColorAt(0.0, QColor( 38,  28,  82));
            cardGrad.setColorAt(0.5, QColor( 30,  22,  66));
            cardGrad.setColorAt(1.0, QColor( 26,  18,  58));
        } else {
            cardGrad.setColorAt(0.0, pal.surface.lighter(115));
            cardGrad.setColorAt(0.5, pal.surface);
            cardGrad.setColorAt(1.0, pal.surface.darker(110));
        }
        p.setBrush(cardGrad);
        p.setPen(Qt::NoPen);
        p.drawPath(path);

        // Hover shimmer: neon sweep across card
        if (hov > 0.0) {
            QLinearGradient shimmer(r.topLeft(), r.bottomRight());
            shimmer.setColorAt(0.0, QColor(180, 100, 255, int(25 * hov)));
            shimmer.setColorAt(0.5, QColor(100, 140, 255, int(15 * hov)));
            shimmer.setColorAt(1.0, QColor( 60, 200, 255, int(10 * hov)));
            p.setBrush(shimmer);
            p.drawPath(path);
        }

        // Top specular highlight (frosted glass edge)
        QLinearGradient spec(r.topLeft(), r.topLeft() + QPointF(0, 20));
        spec.setColorAt(0, QColor(255, 255, 255, int(18 + 14 * hov)));
        spec.setColorAt(1, Qt::transparent);
        p.setBrush(spec);
        p.drawPath(path);

    } else {
        // Light / Dark: flat surface with subtle hover tint
        QColor fill = m_checked ? pal.cardSelected : pal.surface;
        p.setBrush(fill);
        p.setPen(Qt::NoPen);
        p.drawPath(path);

        if (hov > 0.0 && !m_checked) {
            QColor shimmer = pal.accent;
            shimmer.setAlpha(int(12 * hov));
            p.setBrush(shimmer);
            p.drawPath(path);
        }
    }

    // ── Border ────────────────────────────────────────────────────────────────
    if (m_checked) {
        if (isMid) {
            // Glowing gradient border for midnight
            // Qt doesn't do gradient pen, so draw as filled ring
            QPainterPath outer, inner;
            outer.addRoundedRect(r, radius, radius);
            inner.addRoundedRect(r.adjusted(1.5, 1.5, -1.5, -1.5), radius - 1.5, radius - 1.5);
            QPainterPath ring = outer.subtracted(inner);

            QLinearGradient borderGrad(r.topLeft(), r.bottomRight());
            borderGrad.setColorAt(0.0, QColor(200, 120, 255, 255));
            borderGrad.setColorAt(0.5, QColor(130,  80, 255, 255));
            borderGrad.setColorAt(1.0, QColor( 80, 160, 255, 255));
            p.setBrush(borderGrad);
            p.setPen(Qt::NoPen);
            p.drawPath(ring);
        } else {
            p.setBrush(Qt::NoBrush);
            p.setPen(QPen(pal.cardBorderSelected, 1.6));
            p.drawPath(path);
        }
    } else {
        QColor bc = pal.border;
        if (isMid) {
            // Subtle glowing border on hover
            bc.setAlpha(int(80 + 100 * hov));
            if (hov > 0.5) {
                // gradient hint on hover
                QPainterPath outer, inner;
                outer.addRoundedRect(r, radius, radius);
                inner.addRoundedRect(r.adjusted(1, 1, -1, -1), radius - 1, radius - 1);
                QPainterPath ring = outer.subtracted(inner);
                QLinearGradient bg2(r.topLeft(), r.bottomRight());
                bg2.setColorAt(0, QColor(160,  80, 255, int(120 * hov)));
                bg2.setColorAt(1, QColor( 80, 160, 255, int( 80 * hov)));
                p.setBrush(bg2);
                p.setPen(Qt::NoPen);
                p.drawPath(ring);
            } else {
                p.setBrush(Qt::NoBrush);
                p.setPen(QPen(bc, 1.0));
                p.drawPath(path);
            }
        } else {
            bc.setAlpha(int(140 + 115 * hov));
            p.setBrush(Qt::NoBrush);
            p.setPen(QPen(bc, 1.0));
            p.drawPath(path);
        }
    }

    // ── Unavailable overlay ───────────────────────────────────────────────────
    if (!m_available) {
        // Dim overlay
        p.setBrush(QColor(0, 0, 0, 80));
        p.setPen(Qt::NoPen);
        p.drawPath(path);

        // "N/A" badge top-right
        const QRectF badge(r.right() - 34, r.top() + 6, 30, 18);
        p.setBrush(QColor(100, 100, 110, 200));
        p.setPen(Qt::NoPen);
        p.drawRoundedRect(badge, 5, 5);
        p.setPen(QColor(200, 200, 210, 220));
        QFont bf = ThemeManager::fontCaption();
        bf.setPixelSize(9);
        bf.setWeight(QFont::Bold);
        p.setFont(bf);
        p.drawText(badge, Qt::AlignCenter, "N/A");
        return;
    }

    // ── Check badge ───────────────────────────────────────────────────────────
    if (m_checked) {
        const int    bSize = 22;
        const QRectF badge(r.right() - bSize - 7, r.top() + 7, bSize, bSize);

        if (isMid) {
            // Outer glow ring — gradient
            QRadialGradient glowRing(badge.center(), bSize * 1.4);
            glowRing.setColorAt(0.0, QColor(160, 80, 255, 80));
            glowRing.setColorAt(0.5, QColor(100, 80, 255, 40));
            glowRing.setColorAt(1.0, Qt::transparent);
            p.setBrush(glowRing);
            p.setPen(Qt::NoPen);
            p.drawEllipse(badge.adjusted(-8, -8, 8, 8));

            // Gradient badge fill
            QRadialGradient badgeGrad(badge.center() - QPointF(2, 2), bSize * 0.8);
            badgeGrad.setColorAt(0, QColor(180, 110, 255));
            badgeGrad.setColorAt(1, QColor(110,  60, 235));
            p.setBrush(badgeGrad);
            p.drawEllipse(badge);

            // Thin glowing ring around badge
            p.setBrush(Qt::NoBrush);
            p.setPen(QPen(QColor(220, 180, 255, 180), 1.0));
            p.drawEllipse(badge);
        } else {
            QColor glowCol = pal.checkBadge;
            glowCol.setAlpha(50);
            p.setBrush(glowCol);
            p.setPen(Qt::NoPen);
            p.drawEllipse(badge.adjusted(-3, -3, 3, 3));
            p.setBrush(pal.checkBadge);
            p.drawEllipse(badge);
        }

        // Tick (same for all themes)
        p.setPen(QPen(Qt::white, 2.2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        QPointF ctr = badge.center();
        p.drawLine(ctr + QPointF(-4.5,  0.5), ctr + QPointF(-1.5,  3.5));
        p.drawLine(ctr + QPointF(-1.5,  3.5), ctr + QPointF( 5.0, -3.5));
    }
}
