#pragma once
#include "ImageData.h"
#include <QLabel>
#include <QWidget>
#include "ui_ImagePropertiesWidget.h"

class ImagePropertiesWidget : public QWidget
{
    Q_OBJECT

public:
    ImagePropertiesWidget(QWidget *parent = Q_NULLPTR);
    ~ImagePropertiesWidget();

public slots:
    void setProperties(const ImagePropertiesList&);

private:
    Ui::ImagePropertiesWidget ui;
    QLabel *empty_label_ = nullptr;
};
