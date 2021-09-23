// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
//
// Anno Labeling Tool
// 2020-2021 (c) urobots GmbH, https://urobots.io

#include "stdafx.h"
#include "ImageSettingsWidget.h"

ImageSettingsWidget::ImageSettingsWidget(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
}

void ImageSettingsWidget::Init(ImageModel *image) {
    image_ = image;

    connect(ui.grayscale_checkBox, &QCheckBox::stateChanged, image, &ImageModel::set_grayscale);
    connect(ui.brightness_horizontalSlider, &QSlider::valueChanged, image, &ImageModel::set_brightness);
    connect(ui.brightness_spinBox, QOverload<int>::of(&QSpinBox::valueChanged), image, &ImageModel::set_brightness);    
    connect(ui.reset_brightness_toolButton, &QPushButton::clicked, this, &ImageSettingsWidget::ResetBrightness);
    connect(image, &ImageModel::brightness_changed, ui.brightness_horizontalSlider, &QSlider::setValue);
    connect(image, &ImageModel::brightness_changed, ui.brightness_spinBox, &QSpinBox::setValue);

    connect(ui.contrast_horizontalSlider, &QSlider::valueChanged, image, &ImageModel::set_contrast);
    connect(ui.contrast_spinBox, QOverload<int>::of(&QSpinBox::valueChanged), image, &ImageModel::set_contrast);
    connect(ui.reset_contrast_toolButton, &QPushButton::clicked, this, &ImageSettingsWidget::ResetContrast);
    connect(image, &ImageModel::contrast_changed, ui.contrast_spinBox, &QSpinBox::setValue);
    connect(image, &ImageModel::contrast_changed, ui.contrast_horizontalSlider, &QSlider::setValue);

    connect(ui.exr_correction_doubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), image, &ImageModel::set_exr_correction);
}

ImageSettingsWidget::~ImageSettingsWidget() {
}

void ImageSettingsWidget::ResetContrast() {
    image_->set_contrast(0);
}

void ImageSettingsWidget::ResetBrightness() {
    image_->set_brightness(0);
}

