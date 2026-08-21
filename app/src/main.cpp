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
    app.setApplicationVersion(PROJECT_VERSION);
    app.setOrganizationName("Rigset");

    // Set default font immediately to suppress Qt's "QFont::fromString: Invalid
    // description '(empty)'" warning that fires when widgets query the font
    // before one has been explicitly set.
    app.setFont(ThemeManager::fontBody());

    // Bootstrap ThemeManager — Light is the default
    TM().setTheme(ThemeManager::Theme::Light);

    MainWindow window;

    if (auto *screen = QApplication::primaryScreen()) {
        QRect sg = screen->availableGeometry();
        window.move(sg.center() - window.rect().center());
    }

    window.show();

    // Check for updates — blocking overlay if newer version found
    UpdateChecker *updater = new UpdateChecker(&app);
    QObject::connect(updater, &UpdateChecker::updateRequired,
                     &window, [&window, updater](const QString &ver, const QString &url) {
        UpdateChecker::showOverlay(&window, ver, url, updater);
    });
    updater->check();

    return app.exec();
}
