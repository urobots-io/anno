#pragma once
#include "LabelDefinition.h"
#include <QJsonObject>

std::shared_ptr<LabelDefinition> DeserializeLabelDefinition(const QJsonObject &, QStringList& errors);
QJsonObject Serialize(std::shared_ptr<LabelDefinition>);
