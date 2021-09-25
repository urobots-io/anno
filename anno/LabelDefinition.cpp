// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
//
// Anno Labeling Tool
// 2020-2021 (c) urobots GmbH, https://urobots.io

#include "LabelDefinition.h"
#include "FileModel.h"
#include "PropertyDatabase.h"
#include <QRegularExpression>

using namespace std;

LabelDefinition::LabelDefinition(LabelType type, QObject *parent)
: QObject(parent)
, value_type(type) {
}

LabelDefinition::~LabelDefinition() {
}

std::shared_ptr<LabelCategory> LabelDefinition::CreateCategory(std::shared_ptr<LabelDefinition> definition, int value, const QString & name, const QColor & color) {
    auto cat = shared_ptr<LabelCategory>(new LabelCategory(definition, value, name, color));
    definition->categories.push_back(cat);
    return cat;
}

std::shared_ptr<LabelCategory> LabelDefinition::GetCategory(int value) const {
    for (auto c : categories) {
        if (c->get_value() == value) {
            return c;
        }
    }
    return {};
}

void LabelDefinition::set_rendering_script(const QString script) {
    rendering_script_ = script;
    emit Changed();
}

void LabelDefinition::set_description(const QString description) {
    description_ = description;
    emit Changed();
}

void LabelDefinition::ConnectProperty(LabelProperty& prop, const std::string & name, bool inject_my_value) const {
    auto it = shared_properties.find(name);
    if (it != shared_properties.end()) {
        prop.Connect(it->second, inject_my_value);
    }
    else {
        prop.Disconnect();
    }
}

double LabelDefinition::GetSharedPropertyValue(const std::string & name, double default_value) const {
    auto it = shared_properties.find(name);
    if (it != shared_properties.end()) {
        auto def = it->second;
        auto db_value = PropertyDatabase::Instance().GetCurrentValue(def->name, default_value);
        return def->FromDatabaseValue(db_value);
    }
    return default_value;
}

set<int> LabelDefinition::GetMissingIndexes(const set<int>& existing_indexes) const {
    set<int> result;
    for (int i = 0; i < int(shared_labels.size()); ++i) {
        if (!existing_indexes.count(i)) {
            result.insert(i);
        }
    }
    return result;
}

bool LabelDefinition::AllowedForFile(FileModel* file, int shared_index) const {
    if (!file) {
        return false;
    }

    if (!filename_filter.size()) {
        return true;
    }

    auto filename = file->get_id();
    if (shared_index < 0) {
        for (int i = 0; i < int(filename_filter.size()); ++i) {
            if (AllowedForFilename(filename, i)) {
                return true;
            }
        }
        return false;
    }
    else {
        return AllowedForFilename(filename, shared_index);
    }
}

bool LabelDefinition::AllowedForFilename(QString filename, int shared_index) const {
    int index = min<int>(shared_index, int(filename_filter.size()) - 1);
    QRegularExpression re(filename_filter[index].c_str());
    QRegularExpressionMatch match = re.match(filename);
    return match.hasMatch();
}
