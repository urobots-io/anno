#pragma once
#include <QWidget>
#include "ui_ProjectSettingsWidget.h"

class ApplicationModel;

class ProjectSettingsWidget : public QWidget
{
    Q_OBJECT

public:
    ProjectSettingsWidget(QWidget *parent=nullptr);
    ~ProjectSettingsWidget() override;

    void Init(ApplicationModel *model);

public slots:
    void OnTextChanged();
    void OnProjectScriptChanged(QString);
    void OnEditDefinitions();

private:
    Ui::ProjectSettingsWidget ui;
    ApplicationModel *model_ = nullptr;
    QString script_;
};
