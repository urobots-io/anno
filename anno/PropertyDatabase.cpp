#include "PropertyDatabase.h"

PropertyDatabase& PropertyDatabase::Instance() {
    static PropertyDatabase instance;
    return instance;
}

double PropertyDatabase::GetCurrentValue(const std::string & name, double default_value) {
    if (properties_.count(name)) {
        return properties_[name]->value;
    }
    else {
        return default_value;
    }
}

std::shared_ptr<PropertyDatabase::Value> PropertyDatabase::GetSharedValue(const std::string & name, double init_value, bool inject_init_value) {
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