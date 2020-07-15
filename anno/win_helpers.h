#pragma once

namespace win_helpers {

/// Shows a native OS error box. Works even before Qt is initialized.
void ShowStartupErrorBox(const char * appName, const char * message);

bool CreateApplicationMutex(
    const char * app_name,
    const char * mutex_name,
    bool show_message_box);

}