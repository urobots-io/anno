#pragma once
#include <QDialog>
#include "ui_ProjectDefinitionsDialog.h"

class ApplicationModel;

class ProjectDefinitionsDialog : public QDialog
{
    Q_OBJECT

public:
    ProjectDefinitionsDialog(ApplicationModel *model, QWidget *parent = Q_NULLPTR);
    ~ProjectDefinitionsDialog();

    void keyPressEvent(QKeyEvent *e) override;


public slots:
    void OnApply();
    void OnOk();	

private:
	void Apply(bool close_if_success);

private:
    Ui::ProjectDefinitionsDialog ui;
    ApplicationModel* model_ = nullptr;
};
