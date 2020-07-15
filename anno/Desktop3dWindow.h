#pragma once
#include <QWidget>
#include "ui_Desktop3dWindow.h"

class ApplicationModel;
class FileModel;

class Desktop3dWindow : public QWidget
{
    Q_OBJECT

public:
    Desktop3dWindow(ApplicationModel *model, QWidget *parent = Q_NULLPTR);
    ~Desktop3dWindow();    

public slots:
    void OnSelectedImageFileChanged(std::shared_ptr<FileModel> file_model);
    void OnVertexSelected(bool selected, QVector3D position, QVector3D color);

private:
    void UpdateWindowTitle();

private:
    Ui::Desktop3dWindow ui;
    ApplicationModel *model_;
};
