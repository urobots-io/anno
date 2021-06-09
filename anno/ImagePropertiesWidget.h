#pragma once

#include <QWidget>
#include "ui_ImagePropertiesWidget.h"

class ImagePropertiesWidget : public QWidget
{
    Q_OBJECT

public:
    ImagePropertiesWidget(QWidget *parent = Q_NULLPTR);
    ~ImagePropertiesWidget();

public slots:
    void setProperties(QVariantMap);

private:
    Ui::ImagePropertiesWidget ui;
};
