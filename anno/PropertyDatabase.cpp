// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
//
// Anno Labeling Tool
// 2020-2021 (c) urobots GmbH, https://urobots.io

#include "PropertyDatabase.h"

PropertyDatabase& PropertyDatabase::Instance() {
    static PropertyDatabase instance;
    return instance;
}

int PropertyDatabase::GetStateIndex() const { 
    return state_index_; 
}

void PropertyDatabase::Modify() { 
    state_index_++; 
}

void PropertyDatabase::Clear() {
    properties_.clear();
}

double PropertyDatabase::GetCurrentValue(const QString & name, double default_value) {
    if (properties_.count(name)) {
        return properties_[name]->value;
    }
    else {
        return default_value;
    }
}

std::shared_ptr<PropertyDatabase::Value> PropertyDatabase::GetSharedValue(const QString & name, double init_value, bool inject_init_value) {
    if (properties_.count(name)) {
        auto value = properties_[name];
        if (inject_init_value) {
            value->value = init_value;
            value->iparam = 0;
            value->update_counter++;
            Modify();
        }
        return value;
    }
    else {
        auto value = std::make_shared<Value>();
        value->value = init_value;
        properties_[name] = value;
        return value;
    }
}
