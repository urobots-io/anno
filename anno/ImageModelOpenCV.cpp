// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
//
// Anno Labeling Tool
// 2020-2022 (c) urobots GmbH, https://urobots.io

#include "ImageModel.h"
#ifdef ANNO_USE_OPENCV

ImageModelOpenCV::ImageModelOpenCV(QObject *parent)
    : ImageModel(parent)
{    
    connect(this, &ImageModelOpenCV::brightness_changed, this, &ImageModelOpenCV::RebuildPixmap);
    connect(this, &ImageModelOpenCV::contrast_changed, this, &ImageModelOpenCV::RebuildPixmap);
    connect(this, &ImageModelOpenCV::gamma_changed, this, &ImageModelOpenCV::RebuildPixmap);
}

ImageModelOpenCV::~ImageModelOpenCV()
{
}


/*
Mat::type values for debugging purposes:
+--------+----+----+----+----+------+------+------+------+
|        | C1 | C2 | C3 | C4 | C(5) | C(6) | C(7) | C(8) |
+--------+----+----+----+----+------+------+------+------+
| CV_8U  |  0 |  8 | 16 | 24 |   32 |   40 |   48 |   56 |
| CV_8S  |  1 |  9 | 17 | 25 |   33 |   41 |   49 |   57 |
| CV_16U |  2 | 10 | 18 | 26 |   34 |   42 |   50 |   58 |
| CV_16S |  3 | 11 | 19 | 27 |   35 |   43 |   51 |   59 |
| CV_32S |  4 | 12 | 20 | 28 |   36 |   44 |   52 |   60 |
| CV_32F |  5 | 13 | 21 | 29 |   37 |   45 |   53 |   61 |
| CV_64F |  6 | 14 | 22 | 30 |   38 |   46 |   54 |   62 |
+--------+----+----+----+----+------+------+------+------+
*/
namespace {
template<class T, int N>
std::vector<float> GetChannelValuesForType(cv::Mat &m, int x, int y) {
    std::vector<float> result;
    auto p = (T*)&m.at<cv::Vec<T, N>>(y, x);
    for (int i = 0; i < m.channels(); ++i, ++p) {
        result.insert(result.begin(), *p);
    }
    return result;
}

template<int N>
std::vector<float> GetChannelValues(cv::Mat &m, int x, int y) {
    switch (m.depth()) {
    default:
        return{};
    case CV_8U: return GetChannelValuesForType<uint8_t, N>(m, x, y);
    case CV_8S: return GetChannelValuesForType<int8_t, N>(m, x, y);
    case CV_16U: return GetChannelValuesForType<uint16_t, N>(m, x, y);
    case CV_16S: return GetChannelValuesForType<int16_t, N>(m, x, y);
    case CV_32S: return GetChannelValuesForType<int32_t, N>(m, x, y);
    case CV_32F: return GetChannelValuesForType<float, N>(m, x, y);
    case CV_64F: return GetChannelValuesForType<double, N>(m, x, y);
    }
}
}

void ImageModelOpenCV::RebuildPixmapInternal() {
    auto type = image_.type();
    uchar depth = type & CV_MAT_DEPTH_MASK;
    uchar chans = 1 + (type >> CV_CN_SHIFT);

    double max_value = 1.0;
    switch (depth) {
    case CV_8U: max_value = 255; break;
    case CV_16U: max_value = 65535; break;
    case CV_32F:
    default:
        break;
    }

    cv::Mat result;
    image_.convertTo(result, CV_32F);

    if (get_grayscale() && result.channels() > 1) {
        cv::cvtColor(result, result, cv::COLOR_BGR2GRAY);
    }   
   
    auto brightness = get_brightness();
    auto contrast = get_contrast();
    auto gamma = get_gamma();
    
    result /= max_value;

    cv::pow(result, gamma, result);
    
    result = result * contrast + brightness;

    QImage image;
    if (!result.empty()) {
        if (result.channels() == 1) {
            // Currently we need this image only to show, so convert opecnv to color, because this is easy
            cv::cvtColor(result, result, cv::COLOR_GRAY2RGB);
        }
        result.convertTo(result, CV_8U, 255);
        image = QImage( // image that references the existing data
            result.data,
            result.cols,
            result.rows,
            static_cast<int>(result.step1()),
            QImage::Format_RGB888);

        image = image.rgbSwapped(); // swap R & B channels, make a deep copy. 
    }
    
    pixmap_ = QPixmap::fromImage(image);
}

std::vector<float> ImageModelOpenCV::GetBackgroundPixelValues(int x, int y) {
    std::vector<float> result;
    if ((unsigned)x >= (unsigned)image_.cols ||
        (unsigned)y >= (unsigned)image_.rows)
        return result;

    switch (image_.channels()) {
    case 1: return GetChannelValues<1>(image_, x, y);
    case 2: return GetChannelValues<2>(image_, x, y);
    case 3: return GetChannelValues<3>(image_, x, y);
    default:
        return{};
    }
}

#endif

