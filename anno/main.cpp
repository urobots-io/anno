#include "MainWindow.h"
#include "win_helpers.h"
#include "product_info.h"
#include "ProjectDefinitionsDialog.h"
#include "settings.h"
#include <QtWidgets/QApplication>

namespace {
    const char *APP_MUTEX_NAME = "C8F8B7AE-2E70-4B3D-BD87-59D8C74CBA72";
    const char *app_name = ANNO_PRODUCT_NAME;
}

int main(int argc, char *argv[]) {
    if (!win_helpers::CreateApplicationMutex(app_name, APP_MUTEX_NAME, true)) {
        return -1;
    }

	QApplication a(argc, argv);
	
	urobots::qt_helpers::settings::Initialize("anno");

	MainWindow w;
	w.showMaximized();    

    // wait until window is maximized
    a.processEvents();

	if (argc > 1) {
		w.OpenProject(QString::fromLatin1(argv[1]));
	}

#if (0)
    // ap: test
    ProjectDefinitionsDialog dialog(&w.GetModel(), &w);
    dialog.exec();
#endif

	return a.exec();
}
