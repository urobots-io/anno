#pragma once
#include <QJsonObject>
#include <QObject>
#include "RecentActionsList.h"

namespace urobots {
namespace qt_helpers {
namespace settings {

// Allows to store QSettings
void Initialize(QString app_name);

void WriteObjectProperties(QObject*, QString prefix);
void ReadObjectProperties(QObject*, QString prefix);

}
}
}