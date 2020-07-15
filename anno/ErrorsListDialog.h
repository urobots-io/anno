#pragma once
#include <QWidget>
#include "ui_ErrorsListDialog.h"

class ErrorsListDialog : public QDialog
{
    Q_OBJECT

public:
    ErrorsListDialog(QStringList errors, QWidget *parent = Q_NULLPTR);
    ~ErrorsListDialog();

private:
    Ui::ErrorsListDialog ui;
};
