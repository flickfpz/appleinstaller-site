#pragma once
#include <QMainWindow>
#include <QScrollArea>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QMap>
#include <QVector>
#include <QWidget>

#include "Theme.h"       // must come before ThemePicker uses ThemeManager::Theme
#include "OsDetect.h"
#include "AppData.h"
#include "AppCard.h"
#include "CategoryPanel.h"
#include "ProgressOverlay.h"

class GradientBackground;  // defined in MainWindow.cpp

// ── ThemePicker ───────────────────────────────────────────────────────────────
// Three animated pill-buttons in the header that hot-swap the theme.
class ThemePicker : public QWidget
{
    Q_OBJECT
public:
    explicit ThemePicker(QWidget *parent = nullptr);

public slots:
    void applyTheme();

protected:
    void paintEvent(QPaintEvent *) override;

private:
    struct Swatch {
        QPushButton        *btn   = nullptr;
        ThemeManager::Theme themeId;
        QColor              dot;
        QString             label;

        Swatch() : themeId(ThemeManager::Theme::Light) {}
        Swatch(QPushButton *b, ThemeManager::Theme t, const QColor &d, const QString &l)
            : btn(b), themeId(t), dot(d), label(l) {}
    };
    QVector<Swatch> m_swatches;
    void updateActive();
};

// ── MainWindow ────────────────────────────────────────────────────────────────
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

protected:
    void resizeEvent(QResizeEvent *) override;

public slots:
    void applyTheme();

private slots:
    void onCategorySelected(const QString &category);
    void onSearchChanged(const QString &text);
    void onCardSelectionChanged(const QString &id, bool selected);
    void onInstallClicked();
    void onSelectAll();
    void onDeselectAll();
    void refreshInstallButton();

private:
    void buildHeader(QWidget *parent, QVBoxLayout *layout);
    void buildGrid();
    void applyFilter();

    // data
    QVector<AppData>        m_catalogue;
    QMap<QString, AppCard*> m_cards;
    QStringList             m_selectedIds;
    QString                 m_activeCategory;
    QString                 m_searchText;

    // structural UI (no re-theming needed — layout only)
    CategoryPanel      *m_sidebar       = nullptr;
    QScrollArea        *m_scrollArea    = nullptr;
    QWidget            *m_gridContainer = nullptr;
    QGridLayout        *m_grid          = nullptr;
    ProgressOverlay    *m_overlay       = nullptr;
    GradientBackground *m_gradientBg    = nullptr;

    // themed UI widgets (applyTheme touches these)
    QWidget     *m_headerWidget  = nullptr;
    QWidget     *m_bottomBar     = nullptr;
    QWidget     *m_centralWidget = nullptr;
    QLabel      *m_titleLabel    = nullptr;
    QLabel      *m_subtitleLabel = nullptr;
    QLineEdit   *m_searchBox     = nullptr;
    QPushButton *m_installBtn    = nullptr;
    QPushButton *m_selectAllBtn  = nullptr;
    QPushButton *m_deselectBtn   = nullptr;
    QLabel      *m_countLabel    = nullptr;
    ThemePicker *m_themePicker   = nullptr;
    QLabel      *m_osBadge       = nullptr;   // shows detected OS name
};
