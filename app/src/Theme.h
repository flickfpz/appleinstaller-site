#pragma once
#include <QObject>
#include <QColor>
#include <QFont>
#include <QString>
#include <QApplication>

// ─────────────────────────────────────────────────────────────────────────────
// ThemeColors — one complete palette per theme
// ─────────────────────────────────────────────────────────────────────────────
struct ThemeColors {
    // ── Meta ─────────────────────────────────────────────────────────────────
    bool hasGradientBg = false;   // true → widgets should paint gradient bg
    QColor gradA;                 // gradient start color (top-left)
    QColor gradB;                 // gradient mid color
    QColor gradC;                 // gradient end color (bottom-right)

    // Surfaces
    QColor bg;
    QColor surface;
    QColor surfaceAlt;
    QColor surfaceElevated;  // cards hover state, popovers
    QColor glassBg;          // translucent header / sidebar
    QColor glassBorder;      // border on glass panels

    // Borders
    QColor border;
    QColor borderSubtle;

    // Accent (blue by default, same across all themes)
    QColor accent;
    QColor accentHover;
    QColor accentPressed;
    QColor accentDisabled;
    QColor accentText;
    QColor accentSubtle;     // tinted fill for selected cards

    // Semantic
    QColor success;
    QColor successSubtle;
    QColor danger;
    QColor dangerSubtle;
    QColor warning;

    // Text
    QColor textPrimary;
    QColor textSecondary;
    QColor textTertiary;
    QColor textDisabled;
    QColor textOnAccent;

    // Input
    QColor inputBg;
    QColor inputBorder;
    QColor inputBorderFocus;
    QColor inputPlaceholder;

    // Scrollbar
    QColor scrollHandle;
    QColor scrollHandleHover;

    // Special
    QColor dimOverlay;       // progress overlay backdrop
    QColor cardSelected;
    QColor cardBorderSelected;
    QColor checkBadge;
    QColor shadowColor;
};

// ─────────────────────────────────────────────────────────────────────────────
// Built-in palettes
// ─────────────────────────────────────────────────────────────────────────────
namespace Palettes {

inline ThemeColors light()
{
    ThemeColors c;
    c.bg                = { 0xF2, 0xF2, 0xF7 };
    c.surface           = { 0xFF, 0xFF, 0xFF };
    c.surfaceAlt        = { 0xE9, 0xE9, 0xEE };
    c.surfaceElevated   = { 0xFF, 0xFF, 0xFF };
    c.glassBg           = QColor(255, 255, 255, 210);
    c.glassBorder       = QColor(209, 209, 214, 255);
    c.border            = { 0xD1, 0xD1, 0xD6 };
    c.borderSubtle      = { 0xE5, 0xE5, 0xEA };
    c.accent            = {   0, 122, 255 };
    c.accentHover       = {   0,  99, 214 };
    c.accentPressed     = {   0,  80, 185 };
    c.accentDisabled    = { 0xAE, 0xAE, 0xB2 };
    c.accentText        = { 0xFF, 0xFF, 0xFF };
    c.accentSubtle      = QColor(0, 122, 255, 18);
    c.success           = {  52, 199,  89 };
    c.successSubtle     = QColor(52, 199, 89, 20);
    c.danger            = { 255,  59,  48 };
    c.dangerSubtle      = QColor(255, 59, 48, 20);
    c.warning           = { 255, 149,   0 };
    c.textPrimary       = {  29,  29,  31 };
    c.textSecondary     = { 110, 110, 115 };
    c.textTertiary      = { 174, 174, 178 };
    c.textDisabled      = { 199, 199, 204 };
    c.textOnAccent      = { 255, 255, 255 };
    c.inputBg           = { 235, 235, 240 };
    c.inputBorder       = { 209, 209, 214 };
    c.inputBorderFocus  = {   0, 122, 255 };
    c.inputPlaceholder  = { 174, 174, 178 };
    c.scrollHandle      = { 199, 199, 204 };
    c.scrollHandleHover = { 174, 174, 178 };
    c.dimOverlay        = QColor(0, 0, 0, 140);
    c.cardSelected      = QColor(0, 122, 255, 14);
    c.cardBorderSelected= {   0, 122, 255 };
    c.checkBadge        = {   0, 122, 255 };
    c.shadowColor       = QColor(0, 0, 0, 18);
    return c;
}

inline ThemeColors dark()
{
    ThemeColors c;
    c.bg                = {  28,  28,  30 };
    c.surface           = {  44,  44,  46 };
    c.surfaceAlt        = {  36,  36,  38 };
    c.surfaceElevated   = {  58,  58,  60 };
    c.glassBg           = QColor(44, 44, 46, 210);
    c.glassBorder       = QColor(72, 72, 74, 255);
    c.border            = {  72,  72,  74 };
    c.borderSubtle      = {  58,  58,  60 };
    c.accent            = {  10, 132, 255 };
    c.accentHover       = {  40, 150, 255 };
    c.accentPressed     = {   0, 110, 230 };
    c.accentDisabled    = {  72,  72,  74 };
    c.accentText        = { 255, 255, 255 };
    c.accentSubtle      = QColor(10, 132, 255, 30);
    c.success           = {  48, 209,  88 };
    c.successSubtle     = QColor(48, 209, 88, 30);
    c.danger            = { 255,  69,  58 };
    c.dangerSubtle      = QColor(255, 69, 58, 30);
    c.warning           = { 255, 159,  10 };
    c.textPrimary       = { 242, 242, 247 };
    c.textSecondary     = { 174, 174, 178 };
    c.textTertiary      = {  99,  99, 102 };
    c.textDisabled      = {  72,  72,  74 };
    c.textOnAccent      = { 255, 255, 255 };
    c.inputBg           = {  58,  58,  60 };
    c.inputBorder       = {  72,  72,  74 };
    c.inputBorderFocus  = {  10, 132, 255 };
    c.inputPlaceholder  = {  99,  99, 102 };
    c.scrollHandle      = {  72,  72,  74 };
    c.scrollHandleHover = {  99,  99, 102 };
    c.dimOverlay        = QColor(0, 0, 0, 180);
    c.cardSelected      = QColor(10, 132, 255, 28);
    c.cardBorderSelected= {  10, 132, 255 };
    c.checkBadge        = {  10, 132, 255 };
    c.shadowColor       = QColor(0, 0, 0, 60);
    return c;
}

inline ThemeColors midnight()
{
    // ── Deep cosmic violet/indigo — gradient bg + glowing accents ────────────
    ThemeColors c;

    // Gradient bg: deep navy → rich indigo → violet-black
    c.hasGradientBg = true;
    c.gradA         = {   8,   8,  28 };   // near-black navy   (top-left)
    c.gradB         = {  20,  14,  50 };   // deep indigo       (center)
    c.gradC         = {  14,   6,  38 };   // violet-black      (bottom-right)

    c.bg                = {  10,   8,  24 };
    c.surface           = {  22,  18,  48 };   // card fill — dark indigo
    c.surfaceAlt        = {  16,  13,  36 };
    c.surfaceElevated   = {  32,  26,  66 };
    c.glassBg           = QColor(20, 16, 44, 210);
    c.glassBorder       = QColor(90, 70, 140, 180);
    c.border            = {  55,  44,  95 };
    c.borderSubtle      = {  36,  28,  68 };

    // Purple accent — vibrant neon violet
    c.accent            = { 138,  80, 255 };
    c.accentHover       = { 160, 105, 255 };
    c.accentPressed     = { 115,  60, 235 };
    c.accentDisabled    = {  55,  44,  90 };
    c.accentText        = { 255, 255, 255 };
    c.accentSubtle      = QColor(138, 80, 255, 35);

    c.success           = {  50, 220, 120 };
    c.successSubtle     = QColor(50, 220, 120, 35);
    c.danger            = { 255,  80,  90 };
    c.dangerSubtle      = QColor(255, 80, 90, 35);
    c.warning           = { 255, 185,  55 };

    c.textPrimary       = { 235, 228, 255 };   // soft lavender-white
    c.textSecondary     = { 155, 145, 200 };
    c.textTertiary      = {  95,  85, 140 };
    c.textDisabled      = {  60,  52,  95 };
    c.textOnAccent      = { 255, 255, 255 };

    c.inputBg           = {  26,  20,  52 };
    c.inputBorder       = {  55,  44,  95 };
    c.inputBorderFocus  = { 138,  80, 255 };
    c.inputPlaceholder  = {  95,  85, 140 };

    c.scrollHandle      = {  65,  52, 108 };
    c.scrollHandleHover = {  95,  80, 148 };

    c.dimOverlay        = QColor(4, 2, 16, 210);
    c.cardSelected      = QColor(138, 80, 255, 32);
    c.cardBorderSelected= { 138,  80, 255 };
    c.checkBadge        = { 138,  80, 255 };
    c.shadowColor       = QColor(0, 0, 20, 120);   // deep navy shadow
    return c;
}

inline ThemeColors trueBlack()
{
    ThemeColors c;
    c.hasGradientBg = false;

    c.bg                = {   0,   0,   0 };
    c.surface           = {  18,  18,  18 };
    c.surfaceAlt        = {  12,  12,  12 };
    c.surfaceElevated   = {  28,  28,  28 };
    c.glassBg           = QColor(0, 0, 0, 230);
    c.glassBorder       = QColor(40, 40, 40, 255);
    c.border            = {  38,  38,  38 };
    c.borderSubtle      = {  28,  28,  28 };

    c.accent            = { 138,  80, 255 };
    c.accentHover       = { 160, 105, 255 };
    c.accentPressed     = { 115,  60, 235 };
    c.accentDisabled    = {  38,  38,  38 };
    c.accentText        = { 255, 255, 255 };
    c.accentSubtle      = QColor(138, 80, 255, 30);

    c.success           = {  50, 220, 120 };
    c.successSubtle     = QColor(50, 220, 120, 30);
    c.danger            = { 255,  80,  90 };
    c.dangerSubtle      = QColor(255, 80, 90, 30);
    c.warning           = { 255, 185,  55 };

    c.textPrimary       = { 240, 240, 240 };
    c.textSecondary     = { 140, 140, 145 };
    c.textTertiary      = {  80,  80,  85 };
    c.textDisabled      = {  50,  50,  50 };
    c.textOnAccent      = { 255, 255, 255 };

    c.inputBg           = {  14,  14,  14 };
    c.inputBorder       = {  38,  38,  38 };
    c.inputBorderFocus  = { 138,  80, 255 };
    c.inputPlaceholder  = {  80,  80,  85 };

    c.scrollHandle      = {  44,  44,  44 };
    c.scrollHandleHover = {  64,  64,  64 };

    c.dimOverlay        = QColor(0, 0, 0, 220);
    c.cardSelected      = QColor(138, 80, 255, 25);
    c.cardBorderSelected= { 138,  80, 255 };
    c.checkBadge        = { 138,  80, 255 };
    c.shadowColor       = QColor(0, 0, 0, 180);
    return c;
}

} // namespace Palettes

// ─────────────────────────────────────────────────────────────────────────────
// ThemeManager — singleton
// ─────────────────────────────────────────────────────────────────────────────
class ThemeManager : public QObject
{
    Q_OBJECT

public:
    enum class Theme { Light, Dark, Midnight, TrueBlack };

    static ThemeManager &instance()
    {
        static ThemeManager mgr;
        return mgr;
    }

    const ThemeColors &colors() const { return m_colors; }
    Theme currentTheme()        const { return m_theme;  }

    void setTheme(Theme t)
    {
        if (m_theme == t) return;
        m_theme = t;
        switch (t) {
            case Theme::Light:     m_colors = Palettes::light();     break;
            case Theme::Dark:      m_colors = Palettes::dark();      break;
            case Theme::Midnight:  m_colors = Palettes::midnight();  break;
            case Theme::TrueBlack: m_colors = Palettes::trueBlack(); break;
        }
        if (qApp)
            qApp->setStyleSheet(globalStyleSheet());
        emit themeChanged();
    }

    // Convenience aliases — mirror old Theme:: calls
    QColor bg()               const { return m_colors.bg; }
    QColor surface()          const { return m_colors.surface; }
    QColor surfaceAlt()       const { return m_colors.surfaceAlt; }
    QColor border()           const { return m_colors.border; }
    QColor accent()           const { return m_colors.accent; }
    QColor accentHover()      const { return m_colors.accentHover; }
    QColor accentText()       const { return m_colors.accentText; }
    QColor accentSubtle()     const { return m_colors.accentSubtle; }
    QColor textPrimary()      const { return m_colors.textPrimary; }
    QColor textSecondary()    const { return m_colors.textSecondary; }
    QColor textTertiary()     const { return m_colors.textTertiary; }
    QColor danger()           const { return m_colors.danger; }
    QColor success()          const { return m_colors.success; }
    QColor cardSelected()     const { return m_colors.cardSelected; }
    QColor cardBorderSelected() const { return m_colors.cardBorderSelected; }

    // Spacing / radius (static — same across all themes)
    static constexpr int radiusLarge  = 18;
    static constexpr int radiusMedium = 12;
    static constexpr int radiusSmall  =  8;
    static constexpr int cardPadding  = 16;
    static constexpr int cardSpacing  = 14;

    // Fonts
    static QFont fontTitle()
    {
        QFont f;
        f.setFamilies({ "SF Pro Display", ".AppleSystemUIFont", "Helvetica Neue", "Segoe UI Variable Display", "Segoe UI", "sans-serif" });
        f.setPixelSize(26);
        f.setWeight(QFont::Bold);
        f.setLetterSpacing(QFont::AbsoluteSpacing, -0.3);
        return f;
    }
    static QFont fontHeading()
    {
        QFont f;
        f.setFamilies({ "SF Pro Text", ".AppleSystemUIFont", "Helvetica Neue", "Segoe UI Variable Text", "Segoe UI", "sans-serif" });
        f.setPixelSize(14);
        f.setWeight(QFont::DemiBold);
        return f;
    }
    static QFont fontBody()
    {
        QFont f;
        f.setFamilies({ "SF Pro Text", ".AppleSystemUIFont", "Helvetica Neue", "Segoe UI Variable Text", "Segoe UI", "sans-serif" });
        f.setPixelSize(13);
        f.setWeight(QFont::Normal);
        return f;
    }
    static QFont fontCaption()
    {
        QFont f;
        f.setFamilies({ "SF Pro Text", ".AppleSystemUIFont", "Helvetica Neue", "Segoe UI Variable Text", "Segoe UI", "sans-serif" });
        f.setPixelSize(11);
        f.setWeight(QFont::Normal);
        return f;
    }

    QString globalStyleSheet() const
    {
        auto c = [](const QColor &col) { return col.name(QColor::HexArgb); };
        const ThemeColors &p = m_colors;
        return QStringLiteral(R"(
QWidget {
    background-color: %1;
    color: %2;
    font-family: "SF Pro Text", ".AppleSystemUIFont", "Helvetica Neue", "Segoe UI Variable Text", "Segoe UI", sans-serif;
    font-size: 13px;
}
QScrollArea, QScrollArea > QWidget > QWidget {
    background-color: transparent;
    border: none;
}
QScrollBar:vertical {
    background: transparent;
    width: 8px;
    margin: 2px;
}
QScrollBar::handle:vertical {
    background: %3;
    border-radius: 4px;
    min-height: 30px;
}
QScrollBar::handle:vertical:hover { background: %4; }
QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: none; }
QScrollBar:horizontal {
    background: transparent;
    height: 8px;
    margin: 2px;
}
QScrollBar::handle:horizontal {
    background: %3;
    border-radius: 4px;
    min-width: 30px;
}
QScrollBar::handle:horizontal:hover { background: %4; }
QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0; }
QToolTip {
    background-color: %5;
    color: %6;
    border: 1px solid %7;
    border-radius: 8px;
    padding: 5px 10px;
    font-size: 12px;
}
QMenu {
    background-color: %8;
    border: 1px solid %7;
    border-radius: 10px;
    padding: 4px;
}
QMenu::item {
    padding: 6px 20px;
    border-radius: 6px;
    color: %2;
}
QMenu::item:selected { background-color: %9; color: white; }
        )")
        .arg(c(p.bg))
        .arg(c(p.textPrimary))
        .arg(c(p.scrollHandle))
        .arg(c(p.scrollHandleHover))
        .arg(c(p.surfaceElevated))
        .arg(c(p.textPrimary))
        .arg(c(p.border))
        .arg(c(p.surface))
        .arg(c(p.accent));
    }

signals:
    void themeChanged();

private:
    ThemeManager() : m_theme(Theme::Light), m_colors(Palettes::light()) {}
    ThemeManager(const ThemeManager &) = delete;
    ThemeManager &operator=(const ThemeManager &) = delete;

    Theme       m_theme;
    ThemeColors m_colors;
};

// ── Global shorthand ─────────────────────────────────────────────────────────
inline ThemeManager &TM() { return ThemeManager::instance(); }
