#include "ScriptPainter.h"
#include "PropertyDatabase.h"
#include <QJSEngine>

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
    auto cat = label->GetCategory();
	auto script = cat->definition->get_rendering_script();
    if (!script.isEmpty()) {
        this->label = label;

        engine.globalObject().setProperty("text", label->GetText());
        engine.globalObject().setProperty("shared_index", label->GetSharedLabelIndex());
        engine.globalObject().setProperty("category", cat->value);
        engine.globalObject().setProperty("category_name", cat->name);
        engine.globalObject().setProperty("marker_type", cat->definition->type_name);

        auto result = engine.evaluate(script);
        if (result.isError()) {
            qDebug(result.toString().toLatin1());
        }
        
        ResetTransform();

        this->label = nullptr;
    }
    else {
        label->OnPaint(pi);
    }
}