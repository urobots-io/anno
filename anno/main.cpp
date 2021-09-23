// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
//
// Anno Labeling Tool
// 2020-2021 (c) urobots GmbH, https://urobots.io

#include "MainWindow.h"
#include "win_helpers.h"
#include "product_info.h"
#include "ProjectDefinitionsDialog.h"
#include "settings.h"
#include "StartupDialog.h"
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
