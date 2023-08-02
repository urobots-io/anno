#pragma once
#include <QWheelEvent>

namespace QtX {

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)

// Qt 4, 5
inline int GetWheelDelta(QWheelEvent *event) {
    return event->delta();
}

#define QT_BACKGROUND_COLOR_ROLE Qt::BackgroundColorRole
#define QT_MID_BUTTON Qt::MidButton
#define QT_SKIP_EMPTY_PARTS QString::SkipEmptyParts

#else
// Qt 6 and up
inline int GetWheelDelta(QWheelEvent *event) {
    return event->angleDelta().y();
}

#define QT_BACKGROUND_COLOR_ROLE Qt::BackgroundRole
#define QT_MID_BUTTON Qt::MiddleButton
#define QT_SKIP_EMPTY_PARTS Qt::SkipEmptyParts

#endif

}
