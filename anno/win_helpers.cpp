#include "win_helpers.h"
#include <sstream>

namespace win_helpers {

void ShowStartupErrorBox(const char * appName, const char * message) {
#ifndef ANNO_EXCLUDE_WINDOWS_CODE
    std::stringstream ss;
    ss << appName << " startup error";
    MessageBoxA(NULL, message, ss.str().c_str(), MB_ICONERROR);
#endif
}

bool CreateApplicationMutex(
    const char * app_name,
    const char * mutex_name,
    bool show_message_box) {
#ifndef ANNO_EXCLUDE_WINDOWS_CODE
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
