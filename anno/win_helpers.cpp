// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
//
// Anno Labeling Tool
// 2020-2021 (c) urobots GmbH, https://urobots.io

#include "win_helpers.h"
#include <QtGlobal>
#include <sstream>

namespace win_helpers {

void ShowStartupErrorBox(const char * appName, const char * message) {
    Q_UNUSED(appName)
    Q_UNUSED(message)
#ifdef Q_OS_WIN
    std::stringstream ss;
    ss << appName << " startup error";
    MessageBoxA(NULL, message, ss.str().c_str(), MB_ICONERROR);
#endif
}

bool CreateApplicationMutex(
    const char * app_name,
    const char * mutex_name,
    bool show_message_box) {
    Q_UNUSED(app_name)
    Q_UNUSED(mutex_name)
    Q_UNUSED(show_message_box)
#ifdef Q_OS_WIN
    HANDLE mutex = CreateMutexA(NULL, TRUE, mutex_name);
    if (!mutex) {
        if (show_message_box) {
            ShowStartupErrorBox(app_name, "Failed to create mutex!");
        }
        return false;
    }
#endif
    return true;
}

}
