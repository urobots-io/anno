#pragma once
#include <QString>
#include <QVariant>

enum class CustomPropertyType {
    p_unknown = 0,
    p_int,
    p_double,
    p_string,
    p_boolean,
    p_selector
};

struct CustomPropertyDefinition {
    QString id;

    CustomPropertyType type;

    QVariant default_value;

    QStringList cases;
};


inline QString CustomPropertyTypeToString(CustomPropertyType property_type) {
    switch (property_type) {
    default:
    case CustomPropertyType::p_unknown: return "";
    case CustomPropertyType::p_int: return "int";
    case CustomPropertyType::p_double: return "double";
    case CustomPropertyType::p_string: return "string";
    case CustomPropertyType::p_boolean: return "bool";
    case CustomPropertyType::p_selector: return "selector";
    }
}

inline CustomPropertyType CustomPropertyTypeFromString(QString type_name) {
    if (type_name == "int") return CustomPropertyType::p_int;
    else if (type_name == "double") return CustomPropertyType::p_double;
    else if (type_name == "string") return CustomPropertyType::p_string;
    else if (type_name == "bool") return CustomPropertyType::p_boolean;
    else if (type_name == "selector") return CustomPropertyType::p_selector;    
    return CustomPropertyType::p_unknown;
}
