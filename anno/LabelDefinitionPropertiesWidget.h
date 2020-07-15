#pragma once
#include "LabelDefinition.h"
#include "ui_LabelDefinitionPropertiesWidget.h"
#include <QWidget>

class LabelDefinitionPropertiesWidget : public QWidget
{
    Q_OBJECT

public:
    LabelDefinitionPropertiesWidget(QWidget *parent=nullptr);
    ~LabelDefinitionPropertiesWidget();

public slots:
    void SelectDefinition(LabelDefinition*);

private slots:
    void OnDescriptionTextChanged();
    void OnRenderingScriptTextChanged();

private:
    Ui::LabelDefinitionPropertiesWidget ui;
    LabelDefinition *definition_ = nullptr;
};
