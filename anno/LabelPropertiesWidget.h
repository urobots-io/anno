#pragma once
#include "FileModel.h"
#include "Label.h"
#include "ui_LabelPropertiesWidget.h"
#include <QWidget>


class LabelPropertiesWidget : public QWidget
{
	Q_OBJECT

public:
    LabelPropertiesWidget(QWidget *parent = nullptr);
	~LabelPropertiesWidget();

signals:
    void DeleteLabel(std::shared_ptr<Label>);

public slots:
    void OnSelectedLabelChanged(std::shared_ptr<FileModel>, std::shared_ptr<Label>);    
    void SetLabelText(QString);
    void UpdateLabel(std::shared_ptr<Label>);
    void CopyLabelProperties();

private slots:
    void OnDeleteLabel() { emit DeleteLabel(selected_label_); }

private:
	Ui::LabelPropertiesWidget ui;
	std::shared_ptr<Label> selected_label_;
    std::shared_ptr<FileModel> selected_file_;    
};
