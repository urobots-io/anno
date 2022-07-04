#pragma once
#include "ImageModel.h"
#include <QWidget>

#ifdef ANNO_USE_OPENCV
#include "ui_ImageSettingsWidgetOpenCV.h"

class ImageSettingsWidgetOpenCV : public QWidget
{
    Q_OBJECT

public:
    ImageSettingsWidgetOpenCV(QWidget *parent = Q_NULLPTR);
    ~ImageSettingsWidgetOpenCV();

    void Init(ImageModelOpenCV *image);

public slots:
    void ResetContrast();
    void ResetBrightness();
    void ResetGamma();
    void ResetDefaultExr();

    void OnBrightnessSliderChanged(int value);    
    void OnBrightnessValueChanged(double value);

    void OnContrastSliderChanged(int value);
    void OnContrastValueChanged(double value);

    void OnGammaSliderChanged(int value);
    void OnGammaValueChanged(double value);

private:
    double SliderToValue(int ticks, QSlider *slider, QDoubleSpinBox *spinbox);
    int ValueToSlider(double value, QSlider *slider, QDoubleSpinBox *spinbox);

private:
    Ui::ImageSettingsWidgetOpenCV ui;
    ImageModelOpenCV *image_ = nullptr;
};
#endif

