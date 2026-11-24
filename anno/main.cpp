// Anno Labeling Tool
// 2020-2024 (c) urobots GmbH, https://urobots.io/en/portfolio/anno/

#include "MainWindow.h"
#include "win_helpers.h"
#include "product_info.h"
#include "ProjectDefinitionsDialog.h"
#include "settings.h"
#include "StartupDialog.h"
#include <QtWidgets/QApplication>
#include <QStyleHints>

namespace {
    const char *APP_MUTEX_NAME = "C8F8B7AE-2E70-4B3D-BD87-59D8C74CBA72";
    const char *app_name = ANNO_PRODUCT_NAME;
}

int main(int argc, char *argv[]) {
    if (!win_helpers::CreateApplicationMutex(app_name, APP_MUTEX_NAME, true)) {
        return -1;
    }

	QApplication a(argc, argv);

#if defined(Q_OS_WIN) && QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    // Qt 5 uses the legacy GUI font ("MS Shell Dlg 2") as default font, which can be
    // larger than the Windows UI font used by menus and lists. Use the Windows
    // message font (Segoe UI) for all widgets, like Qt 6 does.
    QApplication::setFont(QApplication::font("QMessageBox"));
#endif

#if defined(Q_OS_WIN) && QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    // Qt 6.7+ picks the "windows11" style on Windows 11 (and follows the
    // system dark mode), which changes the palette colors the UI relies on.
    // Keep the classic Qt 5 look.
    QApplication::setStyle("windowsvista");
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
    QGuiApplication::styleHints()->setColorScheme(Qt::ColorScheme::Light);
#endif
#endif

	urobots::qt_helpers::settings::Initialize("anno");

    MainWindow w;
	w.showMaximized();    

    // wait until window is maximized
    a.processEvents();

	if (argc > 1) {
		w.OpenProject(QString::fromLatin1(argv[1]));
	}
    else {
        StartupDialog dialog(&w);
        dialog.exec();
        if (dialog.OpenProject()) {
            w.OnOpenProject();
        }
        else if (dialog.NewProject()) {
            w.OnNewProject();
        }
        else if (!dialog.SelectedProject().isEmpty()) {
            w.OpenProject(dialog.SelectedProject());
        }
    }

	return a.exec();
}
