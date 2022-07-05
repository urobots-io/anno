// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
//
// Anno Labeling Tool
// 2020-2021 (c) urobots GmbH, https://urobots.io

#include "stdafx.h"
#include "ImageSettingsWidgetQt.h"

ImageSettingsWidgetQt::ImageSettingsWidgetQt(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
}

void ImageSettingsWidgetQt::Init(ImageModelQt *image) {
    image_ = image;

    connect(ui.grayscale_checkBox, &QCheckBox::stateChanged, image, &ImageModel::set_grayscale);
    connect(ui.brightness_horizontalSlider, &QSlider::valueChanged, image, &ImageModelQt::set_brightness);
    connect(ui.brightness_spinBox, QOverload<int>::of(&QSpinBox::valueChanged), image, &ImageModelQt::set_brightness);
    connect(ui.reset_brightness_toolButton, &QPushButton::clicked, this, &ImageSettingsWidgetQt::ResetBrightness);
    connect(image, &ImageModelQt::brightness_changed, ui.brightness_horizontalSlider, &QSlider::setValue);
    connect(image, &ImageModelQt::brightness_changed, ui.brightness_spinBox, &QSpinBox::setValue);

    connect(ui.contrast_horizontalSlider, &QSlider::valueChanged, image, &ImageModelQt::set_contrast);
    connect(ui.contrast_spinBox, QOverload<int>::of(&QSpinBox::valueChanged), image, &ImageModelQt::set_contrast);
    connect(ui.reset_contrast_toolButton, &QPushButton::clicked, this, &ImageSettingsWidgetQt::ResetContrast);
    connect(image, &ImageModelQt::contrast_changed, ui.contrast_spinBox, &QSpinBox::setValue);
    connect(image, &ImageModelQt::contrast_changed, ui.contrast_horizontalSlider, &QSlider::setValue);
}

ImageSettingsWidgetQt::~ImageSettingsWidgetQt() {
}

void ImageSettingsWidgetQt::ResetContrast() {
    image_->set_contrast(0);
}

void ImageSettingsWidgetQt::ResetBrightness() {
    image_->set_brightness(0);
}

