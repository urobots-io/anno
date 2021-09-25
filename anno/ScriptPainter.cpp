// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
//
// Anno Labeling Tool
// 2020-2021 (c) urobots GmbH, https://urobots.io

#include "ScriptPainter.h"
#include "PropertyDatabase.h"
#include <QJSEngine>
#include <QDebug>

ScriptPainter::ScriptPainter(QObject* parent) 
    : QObject(parent) {
    qRegisterMetaType<SharedPropertyDefinitionGadget>();
}

QVariant ScriptPainter::Custom(QString property_name) {
    if (label) {
        const auto & cp = label->GetCustomProperties();
        if (cp.contains(property_name)) {
            return cp[property_name];
        }
    }

    return{};
}

double ScriptPainter::Property(QString property_name) {
    if (label) {
        if (auto prop = label->GetProperty(property_name)) {
            return prop->get();
        }
    }
    return 0;
}

SharedPropertyDefinitionGadget ScriptPainter::PropertyDefinition(QString property_name) {
    SharedPropertyDefinitionGadget result;
    if (label) {
        if (auto prop = label->GetProperty(property_name)) {
            if (auto def = prop->definition()) {
                result.a = def->a;
                result.b = def->b;
            }
        }

    }
    return result;
}

void ScriptPainter::RenderLabel(QJSEngine & engine, Label *label) {
    auto definition = label->GetDefinition();
	auto script = definition ? definition->get_rendering_script() : QString();
    if (!script.isEmpty()) {
        this->label = label;

        auto cat = label->GetCategory();
        engine.globalObject().setProperty("text", label->GetText());
        engine.globalObject().setProperty("shared_index", label->GetSharedLabelIndex());
        engine.globalObject().setProperty("category", cat->get_value());
        engine.globalObject().setProperty("category_name", cat->get_name());
        engine.globalObject().setProperty("marker_type", definition->type_name);

        auto result = engine.evaluate(script);
        if (result.isError()) {
            qDebug() << result.toString();
        }
        
        ResetTransform();

        this->label = nullptr;
    }
    else {
        label->OnPaint(pi, pf);
    }
}
