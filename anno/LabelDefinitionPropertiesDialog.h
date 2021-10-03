#ifndef LABELDEFINITIONPROPERTIESDIALOG_H
#define LABELDEFINITIONPROPERTIESDIALOG_H

#include "LabelDefinitionsTreeModel.h"
#include "ui_LabelDefinitionPropertiesDialog.h"
#include <QDialog>
#include <memory>

class LabelDefinitionPropertiesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LabelDefinitionPropertiesDialog(std::shared_ptr<LabelDefinition>, std::shared_ptr<LabelDefinitionsTreeModel>, QWidget *parent = nullptr);
    ~LabelDefinitionPropertiesDialog();

private slots:
    void onSharedLabelsCountChanged(int);

private:
    Ui::LabelDefinitionPropertiesDialog ui;
    std::shared_ptr<LabelDefinition> definition_;
    std::shared_ptr<LabelDefinitionsTreeModel> definitions_;
};

#endif // LABELDEFINITIONPROPERTIESDIALOG_H
