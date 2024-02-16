// Anno Labeling Tool
// 2020-2022 (c) urobots GmbH, https://urobots.io

#include "stdafx.h"
#include "ImageSettingsWidgetOpenCV.h"

#ifdef ANNO_USE_OPENCV

ImageSettingsWidgetOpenCV::ImageSettingsWidgetOpenCV(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
}

void ImageSettingsWidgetOpenCV::Init(ImageModelOpenCV *image) {
    image_ = image;

    connect(ui.reset_brightness_toolButton, &QPushButton::clicked, this, &ImageSettingsWidgetOpenCV::ResetBrightness);
    connect(ui.reset_contrast_toolButton, &QPushButton::clicked, this, &ImageSettingsWidgetOpenCV::ResetContrast);
    connect(ui.reset_gamma_toolButton, &QPushButton::clicked, this, &ImageSettingsWidgetOpenCV::ResetGamma);
    connect(ui.set_default_exr_correction_toolButton, &QPushButton::clicked, this, &ImageSettingsWidgetOpenCV::ResetDefaultExr);

    connect(ui.grayscale_checkBox, &QCheckBox::stateChanged, image, &ImageModel::set_grayscale);
    
    connect(ui.brightness_horizontalSlider, &QSlider::valueChanged, this, &ImageSettingsWidgetOpenCV::OnBrightnessSliderChanged);
    connect(ui.brightness_doubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), image, &ImageModelOpenCV::set_brightness);        
    connect(image, &ImageModelOpenCV::brightness_changed, this, &ImageSettingsWidgetOpenCV::OnBrightnessValueChanged);    

    connect(ui.contrast_horizontalSlider, &QSlider::valueChanged, this, &ImageSettingsWidgetOpenCV::OnContrastSliderChanged);
    connect(ui.contrast_doubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), image, &ImageModelOpenCV::set_contrast);
    connect(image, &ImageModelOpenCV::contrast_changed, this, &ImageSettingsWidgetOpenCV::OnContrastValueChanged);

    connect(ui.gamma_horizontalSlider, &QSlider::valueChanged, this, &ImageSettingsWidgetOpenCV::OnGammaSliderChanged);
    connect(ui.gamma_doubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), image, &ImageModelOpenCV::set_gamma);
    connect(image, &ImageModelOpenCV::gamma_changed, this, &ImageSettingsWidgetOpenCV::OnGammaValueChanged);

    OnBrightnessValueChanged(image_->get_brightness());
    OnContrastValueChanged(image_->get_contrast());
    OnGammaValueChanged(image_->get_gamma());
}

ImageSettingsWidgetOpenCV::~ImageSettingsWidgetOpenCV() {
}

double ImageSettingsWidgetOpenCV::SliderToValue(int ticks, QSlider *slider, QDoubleSpinBox *spinbox) {
    auto minimum_slider = slider->minimum();
    auto maximum_slider = slider->maximum();

    auto minimum_spinbox = spinbox->minimum();
    auto maximum_spinbox = spinbox->maximum();

    return (ticks >= 0)
        ? (double(ticks) / maximum_slider) * maximum_spinbox
        : (double(ticks) / minimum_slider) * minimum_spinbox;
}

int ImageSettingsWidgetOpenCV::ValueToSlider(double value, QSlider *slider, QDoubleSpinBox *spinbox) {
    auto minimum_slider = slider->minimum();
    auto maximum_slider = slider->maximum();

    auto minimum_spinbox = spinbox->minimum();
    auto maximum_spinbox = spinbox->maximum();

    return (value >= 0)
        ? (double(value) / maximum_spinbox) * maximum_slider
        : (double(value) / minimum_spinbox) * minimum_slider;
}

void ImageSettingsWidgetOpenCV::OnBrightnessSliderChanged(int ticks) {
    image_->set_brightness(SliderToValue(ticks, ui.brightness_horizontalSlider, ui.brightness_doubleSpinBox));
}

void ImageSettingsWidgetOpenCV::OnBrightnessValueChanged(double value) {
    ui.brightness_doubleSpinBox->blockSignals(true);
    ui.brightness_horizontalSlider->blockSignals(true);

    ui.brightness_doubleSpinBox->setValue(value);
    ui.brightness_horizontalSlider->setValue(ValueToSlider(value, ui.brightness_horizontalSlider, ui.brightness_doubleSpinBox));

    ui.brightness_doubleSpinBox->blockSignals(false);
    ui.brightness_horizontalSlider->blockSignals(false);
}

void ImageSettingsWidgetOpenCV::OnContrastSliderChanged(int ticks) {
    image_->set_contrast(SliderToValue(ticks, ui.contrast_horizontalSlider, ui.contrast_doubleSpinBox));
}

void ImageSettingsWidgetOpenCV::OnContrastValueChanged(double value) {
    ui.contrast_doubleSpinBox->blockSignals(true);
    ui.contrast_horizontalSlider->blockSignals(true);

    ui.contrast_doubleSpinBox->setValue(value);
    ui.contrast_horizontalSlider->setValue(ValueToSlider(value, ui.contrast_horizontalSlider, ui.contrast_doubleSpinBox));

    ui.contrast_doubleSpinBox->blockSignals(false);
    ui.contrast_horizontalSlider->blockSignals(false);
}

void ImageSettingsWidgetOpenCV::OnGammaSliderChanged(int ticks) {
    image_->set_gamma(SliderToValue(ticks, ui.gamma_horizontalSlider, ui.gamma_doubleSpinBox));
}

void ImageSettingsWidgetOpenCV::OnGammaValueChanged(double value) {
    ui.gamma_doubleSpinBox->blockSignals(true);
    ui.gamma_horizontalSlider->blockSignals(true);

    ui.gamma_doubleSpinBox->setValue(value);
    ui.gamma_horizontalSlider->setValue(ValueToSlider(value, ui.gamma_horizontalSlider, ui.gamma_doubleSpinBox));

    ui.gamma_doubleSpinBox->blockSignals(false);
    ui.gamma_horizontalSlider->blockSignals(false);
}

void ImageSettingsWidgetOpenCV::ResetContrast() {
    image_->set_contrast(1);
}

void ImageSettingsWidgetOpenCV::ResetBrightness() {
    image_->set_brightness(0);
}
void ImageSettingsWidgetOpenCV::ResetGamma() {
    image_->set_gamma(1);
}

void ImageSettingsWidgetOpenCV::ResetDefaultExr() {
    image_->set_contrast(1);
    image_->set_brightness(0);
    image_->set_gamma(0.4);
}

#endif
