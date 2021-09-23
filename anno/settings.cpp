// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
//
// Anno Labeling Tool
// 2020-2021 (c) urobots GmbH, https://urobots.io

#include <QApplication>
#include <QMetaProperty>
#include <QSettings>
#include <QString>
#include <QStringBuilder>

namespace urobots {
namespace qt_helpers {
namespace settings {

void Initialize(QString app_name) {
    if (auto app = QApplication::instance()) {
        app->setOrganizationName("urobots");
        app->setApplicationName(app_name);
    }
    else {
        throw std::runtime_error("QApplication is not yet created");
    }
}

void WriteObjectProperties(QObject* object, QString prefix) {
    QSettings settings;
    auto meta = object->metaObject();
    for (int i = 0; i < meta->propertyCount(); ++i) {
        auto property = meta->property(i);
        if (strcmp("objectName", property.name())) {
            auto value = property.read(object);
            if (property.isEnumType()) {
                auto key = property.enumerator().valueToKey(value.toInt());
                if (key) {
                    settings.setValue(prefix % property.name(), key);
                    continue;
                }
            }
            settings.setValue(prefix % property.name(), value);
        }
    }
}

void ReadObjectProperties(QObject* object, QString prefix) {
    QSettings settings;
    auto meta = object->metaObject();
    for (int i = 0; i < meta->propertyCount(); ++i) {
        auto property = meta->property(i);
        auto value = settings.value(prefix % property.name());
        if (!value.isNull()) {
            property.write(object, value);
        }
    }
}

}
}
}
