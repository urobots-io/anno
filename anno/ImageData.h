#pragma once
#include <QVariant>
#include <vector>

#ifdef ANNO_USE_OPENCV
#include <opencv2/opencv.hpp>
typedef cv::Mat ImageData;
inline bool IsImageEmpty(const cv::Mat & img) { return img.empty(); }
#else
#include <QImage>
typedef QImage ImageData;
inline bool IsImageEmpty(const QImage& img) { return img.isNull(); }
#endif

struct ImageProperty {
    QString name;
    QVariant value;
};

typedef QVector<ImageProperty> ImagePropertiesList;
