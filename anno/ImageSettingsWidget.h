#pragma once
#include "ImageModel.h"
#include <QWidget>
#include "ui_ImageSettingsWidget.h"

class ImageSettingsWidget : public QWidget
{
    Q_OBJECT

public:
    ImageSettingsWidget(QWidget *parent = Q_NULLPTR);
    ~ImageSettingsWidget();

    void Init(ImageModel *image);

public slots:
    void ResetContrast();
    void ResetBrightness();

private:
    Ui::ImageSettingsWidget ui;
    ImageModel *image_ = nullptr;
};
