#include <QApplication>
#include <QScreen>
#include "MainWindow.h"
#include "Theme.h"
#include "UpdateChecker.h"

int main(int argc, char *argv[])
{
    QApplication::setHighDpiScaleFactorRoundingPolicy(
        Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

    QApplication app(argc, argv);
    app.setApplicationName("Rigset");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("Kiro");

    // Bootstrap ThemeManager — Light is the default
    // Calling setTheme triggers globalStyleSheet() + themeChanged()
    // so all future widgets start with the correct palette.
    TM().setTheme(ThemeManager::Theme::Light);
    app.setFont(ThemeManager::fontBody());

    MainWindow window;

    if (auto *screen = QApplication::primaryScreen()) {
        QRect sg = screen->availableGeometry();
        window.move(sg.center() - window.rect().center());
    }

    window.show();

    // Check for updates — blocking overlay if newer version found
    UpdateChecker *updater = new UpdateChecker(&app);
    QObject::connect(updater, &UpdateChecker::updateRequired,
                     &window, [&window](const QString &ver, const QString &url) {
        UpdateChecker::showOverlay(&window, ver, url);
    });
    updater->check();

    return app.exec();
}
