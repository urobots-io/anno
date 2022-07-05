// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
//
// Anno Labeling Tool
// 2020-2022 (c) urobots GmbH, https://urobots.io

#include "ImageModel.h"
#include "ColorTransformer.h"

ImageModelQt::ImageModelQt(QObject *parent)
    : ImageModel(parent)
{
    connect(this, &ImageModelQt::brightness_changed, this, &ImageModel::RebuildPixmap);
    connect(this, &ImageModelQt::contrast_changed, this, &ImageModel::RebuildPixmap);    
}

ImageModelQt::~ImageModelQt()
{
}

std::vector<float> ImageModelQt::GetBackgroundPixelValues(int x, int y) {
#ifndef ANNO_USE_OPENCV
    if (image_.isNull() ||
            x < 0 || y < 0 ||
            x >= image_.width() || y >= image_.height()) {
        return {};
    }

    auto color = QColor(image_.pixel(QPoint(x, y)));
    return {
        float(color.red()),
        float(color.green()),
        float(color.blue())
    };
#else
    return {};
#endif
}

void ImageModelQt::RebuildPixmapInternal() {
#ifndef ANNO_USE_OPENCV
    int brightness = get_brightness();
    int contrast = get_contrast();

    if (brightness || contrast) {
        ColorTransformer ct(brightness, contrast);

        auto image = image_.convertToFormat(QImage::Format_RGB888);
        for (int y = 0; y < image.height(); ++y) {
            auto p = image.scanLine(y);
            for (int x = 0; x < image.width(); ++x, p += 3) {
                ct.TransformB3(p);
            }
        }

        if (get_grayscale()) {
            pixmap_ = QPixmap::fromImage(image.convertToFormat(QImage::Format_Grayscale8));
        }
        else {
            pixmap_ = QPixmap::fromImage(image);
        }
    }
    else if (get_grayscale()) {
        pixmap_ = QPixmap::fromImage(image_.convertToFormat(QImage::Format_Grayscale8));
    }
    else {
        pixmap_ = QPixmap::fromImage(image_);
    }
#endif
}

