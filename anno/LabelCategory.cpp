#include "LabelCategory.h"

LabelCategory::LabelCategory(LabelDefinition* definition, int value, const QString & name, const QColor & color)
    : definition(definition)
    , value(value)
    , name(name)
    , color(color)
{
}

QColor LabelCategory::GetStandardColor(int index) {
    switch (index % 10) {
    default:
    case 0: return QColor(255, 0, 0);
    case 1: return QColor(0, 255, 0);
    case 2: return QColor(0, 0, 255);
    case 3: return QColor(255, 55, 0);
    case 4: return QColor(255, 0, 255);
    case 5: return QColor(0, 255, 255);
    case 6: return QColor(55, 55, 55);
    case 7: return QColor(255, 100, 100);
    case 8: return QColor(155, 0, 155);
    case 9: return QColor(0, 100, 255);
    }
}

