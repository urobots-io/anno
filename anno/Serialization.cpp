#include "Serialization.h"
#include "LabelFactory.h"
#include "qjson_helpers.h"
#include <QJsonArray>

#define K_DEFINITION_NAME "name"
#define K_DEFINITION_DESCRIPTION "description"
#define K_DEFINITION_VALUE_TYPE "value_type"
#define K_DEFINITION_LINE_WIDTH "line_width"
#define K_DEFINITION_IS_STAMP "stamp"
#define K_DEFINITION_AXIS_LENGTH "axis_length"
#define K_DEFINITION_RENDERING_SCRIPT "rendering_script"
#define K_DEFINITION_STAMP_PARAMETERS "stamp_parameters"
#define K_DEFINITION_CATEGORIES "categories"
#define K_DEFINITION_SHARED "shared"
#define K_DEFINITION_SHARED_COUNT "shared_count"
#define K_DEFINITION_FILENAME_FILTER "filename_filter"
#define K_DEFINITION_SHARED_PROPERTIES "shared_properties"
#define K_DEFINITION_CUSTOM_PROPERTIES "custom_properties"

#define K_DEFINITION_SHARED_PROPERTY_NAME "name"
#define K_DEFINITION_SHARED_PROPERTY_LINEAR_TRANSFORM_A "a"
#define K_DEFINITION_SHARED_PROPERTY_LINEAR_TRANSFORM_B "b"

#define K_CUSTOM_PROP_TYPE "type"
#define K_CUSTOM_PROP_DEFAULT_VALUE "default"
#define K_CUSTOM_PROP_CASES "cases"

#define K_CATEGORY_ID "id"
#define K_CATEGORY_NAME "name"
#define K_CATEGORY_COLOR "color"

std::shared_ptr<LabelDefinition> DeserializeLabelDefinition(const QJsonObject & def_json, QStringList& errors) {
    auto def = std::make_shared<LabelDefinition>();

    if (def_json.contains(K_DEFINITION_DESCRIPTION)) {
        def->set_description(def_json[K_DEFINITION_DESCRIPTION].toString());
    }
    else if (def_json.contains(K_DEFINITION_NAME)) {
        // backward compatibility - transform name into description
        def->set_description(def_json[K_DEFINITION_NAME].toString());
    }
    def->value_type = LabelTypeFromString(def_json[K_DEFINITION_VALUE_TYPE].toString());

    if (def_json.contains(K_DEFINITION_LINE_WIDTH))
        def->line_width = def_json[K_DEFINITION_LINE_WIDTH].toInt();

    if (def_json.contains(K_DEFINITION_IS_STAMP))
        def->is_stamp = def_json[K_DEFINITION_IS_STAMP].toBool();

    int num_shared = 0;
    if (def_json.contains(K_DEFINITION_SHARED))
        num_shared = def_json[K_DEFINITION_SHARED].toBool() ? 1 : 0;

    if (def_json.contains(K_DEFINITION_SHARED_COUNT))
        num_shared = def_json[K_DEFINITION_SHARED_COUNT].toInt();

    if (def_json.contains(K_DEFINITION_FILENAME_FILTER)) {
        auto filter = def_json[K_DEFINITION_FILENAME_FILTER];
        if (filter.isString()) {
            def->filename_filter.push_back(filter.toString().toStdString());
        }
        else if (filter.isArray()) {
            for (const auto &i : filter.toArray()) {
                def->filename_filter.push_back(i.toString().toStdString());
            }
        }
    }

    if (def_json.contains(K_DEFINITION_AXIS_LENGTH)) {
        def->axis_length.clear();
        for (const auto &i : def_json[K_DEFINITION_AXIS_LENGTH].toArray()) {
            def->axis_length.push_back(i.toInt());
        }
    }

    if (def_json.contains(K_DEFINITION_SHARED_PROPERTIES)) {
        auto shared_props = def_json[K_DEFINITION_SHARED_PROPERTIES].toObject();
        for (auto key : shared_props.keys()) {
            auto property_def = std::make_shared<SharedPropertyDefinition>();
            auto jnode = shared_props[key];
            if (!jnode.isObject()) {
                property_def->name = jnode.toString().toStdString();
            }
            else {
                auto jobj = jnode.toObject();
                property_def->name = jobj[K_DEFINITION_SHARED_PROPERTY_NAME].toString().toStdString();
                if (jobj.contains(K_DEFINITION_SHARED_PROPERTY_LINEAR_TRANSFORM_A)) {
                    property_def->a = jobj[K_DEFINITION_SHARED_PROPERTY_LINEAR_TRANSFORM_A].toDouble();
                }
                if (jobj.contains(K_DEFINITION_SHARED_PROPERTY_LINEAR_TRANSFORM_B)) {
                    property_def->b = jobj[K_DEFINITION_SHARED_PROPERTY_LINEAR_TRANSFORM_B].toDouble();
                }
            }
            def->shared_properties[key.toStdString()] = property_def;
        }
    }

    if (def_json.contains(K_DEFINITION_CUSTOM_PROPERTIES)) {
        auto custom_properties = def_json[K_DEFINITION_CUSTOM_PROPERTIES].toObject();
        for (auto key : custom_properties.keys()) {
            auto iobj = custom_properties[key].toObject();
            CustomPropertyDefinition p;
            p.id = key;
            p.default_value = iobj[K_CUSTOM_PROP_DEFAULT_VALUE].toVariant();
            p.type = CustomPropertyTypeFromString(iobj[K_CUSTOM_PROP_TYPE].toString());
            p.cases = ToStringList(iobj[K_CUSTOM_PROP_CASES]);
            def->custom_properties.push_back(p);
        }
    }
    def->stamp_parameters = def_json[K_DEFINITION_STAMP_PARAMETERS].toObject();
    def->set_rendering_script(ArrayToString(def_json[K_DEFINITION_RENDERING_SCRIPT]));

    auto categories = def_json[K_DEFINITION_CATEGORIES].toArray();
    for (int i = 0; i < categories.size(); ++i) {
        auto json = categories[i].toObject();
        auto category = std::make_shared<LabelCategory>();

        category->value = json[K_CATEGORY_ID].toInt();
        category->name = json[K_CATEGORY_NAME].toString();
        category->color = LabelCategory::GetStandardColor(category->value);
        category->definition = def.get();

        auto jcolor = json[K_CATEGORY_COLOR];
        if (!jcolor.isNull())
            category->color.setNamedColor(jcolor.toString());

        def->categories.push_back(category);
    }

    if (categories.size()) {
        for (int i = 0; i < num_shared; ++i) {
            // create label shared between other labels
            auto shared_label = LabelFactory::CreateLabel(def->value_type);
            if (!shared_label) {
                errors << QString("Failed to create label with type \"%0\" for definition \"%1\"")
                    .arg(LabelTypeToString(def->value_type))
                    .arg(def->type_name);
                return {};
            }

            // setup valid shared label index
            shared_label->SetSharedLabelIndex(i);

            // use first category
            shared_label->SetCategory(def->categories[0].get());

            // connect to the database
            shared_label->ConnectSharedProperties(true, false);

            def->shared_labels.push_back(shared_label);
        }
    }

    return def;
}

QJsonObject Serialize(std::shared_ptr<LabelDefinition> def) {
    QJsonObject json;
    json.insert(K_DEFINITION_VALUE_TYPE, QJsonValue::fromVariant(LabelTypeToString(def->value_type)));

    if (def->line_width != LabelDefinition::default_line_width) {
        json.insert(K_DEFINITION_LINE_WIDTH, QJsonValue::fromVariant(def->line_width));
    }

    if (!def->get_description().isEmpty()) {
        json.insert(K_DEFINITION_DESCRIPTION, QJsonValue::fromVariant(def->get_description()));
    }

    QJsonArray categories;

    for (auto c : def->categories) {
        QJsonObject json;
        json.insert(K_CATEGORY_NAME, c->name);
        json.insert(K_CATEGORY_ID, c->value);
        json.insert(K_CATEGORY_COLOR, c->color.name());

        categories.push_back(json);
    }

    json.insert(K_DEFINITION_CATEGORIES, categories);

    if (def->get_rendering_script().size()) {
        json.insert(K_DEFINITION_RENDERING_SCRIPT, ToJsonArray(def->get_rendering_script()));
    }

    if (def->is_stamp) {
        json.insert(K_DEFINITION_IS_STAMP, QJsonValue::fromVariant(true));
    }

    if (def->shared_labels.size() == 1) {
        json.insert(K_DEFINITION_SHARED, QJsonValue::fromVariant(true));
    }
    else if (def->shared_labels.size() > 1) {
        json.insert(K_DEFINITION_SHARED_COUNT, QJsonValue::fromVariant(int(def->shared_labels.size())));
    }

    if (def->filename_filter.size()) {
        QJsonArray filename_filters_array;
        for (const auto &i : def->filename_filter)
            filename_filters_array.push_back(QJsonValue::fromVariant(QString::fromStdString(i)));
        json.insert(K_DEFINITION_FILENAME_FILTER, filename_filters_array);
    }

    if (def->custom_properties.size()) {
        QJsonObject custom_properties;
        for (auto p : def->custom_properties) {
            QJsonObject jp;
            jp.insert(K_CUSTOM_PROP_TYPE, CustomPropertyTypeToString(p.type));
            if (p.default_value.isValid()) {
                jp.insert(K_CUSTOM_PROP_DEFAULT_VALUE, QJsonValue::fromVariant(p.default_value));
            }
            if (p.cases.size()) {
                jp.insert(K_CUSTOM_PROP_CASES, ToJsonArray(p.cases));
            }
            custom_properties.insert(p.id, jp);
        }
        json.insert(K_DEFINITION_CUSTOM_PROPERTIES, custom_properties);
    }

    if (!def->stamp_parameters.isEmpty()) {
        json.insert(K_DEFINITION_STAMP_PARAMETERS, def->stamp_parameters);
    }

    if (def->shared_properties.size()) {
        QJsonObject shared_properties;
        for (auto p : def->shared_properties) {
            if (p.second->IsIdentity()) {
                shared_properties[QString::fromStdString(p.first)] = QString::fromStdString(p.second->name);
            }
            else {
                QJsonObject jprop;
                jprop.insert(K_DEFINITION_SHARED_PROPERTY_NAME, QString::fromStdString(p.second->name));
                jprop.insert(K_DEFINITION_SHARED_PROPERTY_LINEAR_TRANSFORM_A, QJsonValue::fromVariant(p.second->a));
                if (p.second->b != 0) {
                    jprop.insert(K_DEFINITION_SHARED_PROPERTY_LINEAR_TRANSFORM_B, QJsonValue::fromVariant(p.second->b));
                }
                shared_properties[QString::fromStdString(p.first)] = jprop;
            }
        }
        json.insert(K_DEFINITION_SHARED_PROPERTIES, shared_properties);
    }

    if (!def->axis_length.empty()) {
        QJsonArray axis_len;
        for (const auto &i : def->axis_length) {
            axis_len.push_back(QJsonValue::fromVariant(i));
        }
        json.insert(K_DEFINITION_AXIS_LENGTH, axis_len);
    }

    return json;
}
