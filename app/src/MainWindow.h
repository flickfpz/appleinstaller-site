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
#include <QStackedWidget>

#include "Theme.h"
#include "OsDetect.h"
#include "AppData.h"
#include "AppCard.h"
#include "CategoryPanel.h"
#include "ProgressOverlay.h"
#include "WizardPage.h"

class GradientBackground;

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
    void goToPage(int index);
    void onInstallClicked();
    void refreshInstallButton();
    void toggleWizard();
    void toggleUninstall();
    void filterGrid(const QString &category);

private:
    void buildPages();
    void buildGrid();
    void applyFilter();

    QVector<AppData>        m_catalogue;
    QVector<WizardPage*>    m_pages;
    QVector<AppCard*>       m_gridCards;
    QStringList             m_categoryOrder;
    QStringList             m_selectedIds;
    int                     m_currentPage = 0;
    bool                    m_wizardMode  = false;
    bool                    m_uninstallMode = false;

    CategoryPanel      *m_sidebar       = nullptr;
    QStackedWidget     *m_stack         = nullptr;
    ProgressOverlay    *m_overlay       = nullptr;
    GradientBackground *m_gradientBg    = nullptr;
    QScrollArea        *m_gridScroll    = nullptr;

    QWidget     *m_headerWidget  = nullptr;
    QWidget     *m_bottomBar     = nullptr;
    QWidget     *m_centralWidget = nullptr;
    QLabel      *m_titleLabel    = nullptr;
    QLabel      *m_subtitleLabel = nullptr;
    QLineEdit   *m_searchBox     = nullptr;
    QPushButton *m_installBtn    = nullptr;
    QPushButton *m_wizardBtn     = nullptr;
    QPushButton *m_uninstallBtn  = nullptr;
    QLabel      *m_countLabel    = nullptr;
    QLabel      *m_stepLabel     = nullptr;
    ThemePicker *m_themePicker   = nullptr;
    QLabel      *m_osBadge       = nullptr;
};
