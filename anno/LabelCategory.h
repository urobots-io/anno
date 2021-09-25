// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
//
// Anno Labeling Tool
// 2020-2021 (c) urobots GmbH, https://urobots.io

#include <QObject>
#include <QString>
#include <memory>

class LabelDefinition;

class LabelCategory : public QObject {
public:
    LabelCategory(LabelDefinition* definition, int value, const QString & name, const QColor & color);
        
    /// parent definition
    LabelDefinition* definition;

    /// id of the category
    int value;

    /// human readable name
    QString name;

    /// color
    QColor color;

    static QColor GetStandardColor(int index);
};
