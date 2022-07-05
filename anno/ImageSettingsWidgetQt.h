#pragma once
#include "ImageModel.h"
#include <QWidget>
#include "ui_ImageSettingsWidgetQt.h"

class ImageSettingsWidgetQt : public QWidget
{
    Q_OBJECT

public:
    ImageSettingsWidgetQt(QWidget *parent = Q_NULLPTR);
    ~ImageSettingsWidgetQt();

    void Init(ImageModelQt *image);

public slots:
    void ResetContrast();
    void ResetBrightness();

private:
    Ui::ImageSettingsWidgetQt ui;
    ImageModelQt *image_ = nullptr;
};
