#ifndef LABELDEFINITIONPROPERTIESDIALOG_H
#define LABELDEFINITIONPROPERTIESDIALOG_H

#include <QDialog>

namespace Ui {
class LabelDefinitionPropertiesDialog;
}

class LabelDefinitionPropertiesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LabelDefinitionPropertiesDialog(QWidget *parent = nullptr);
    ~LabelDefinitionPropertiesDialog();

private:
    Ui::LabelDefinitionPropertiesDialog *ui;
};

#endif // LABELDEFINITIONPROPERTIESDIALOG_H
