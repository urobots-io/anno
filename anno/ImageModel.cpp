#include "ImageModel.h"
#include "ColorTransformer.h"

ImageModel::ImageModel(QObject *parent)
    : QObject(parent)
{
    connect(this, &ImageModel::brightness_changed, this, &ImageModel::RebuildPixmap);
    connect(this, &ImageModel::contrast_changed, this, &ImageModel::RebuildPixmap);
    connect(this, &ImageModel::grayscale_changed, this, &ImageModel::RebuildPixmap);
    connect(this, &ImageModel::exr_correction_changed, this, &ImageModel::RebuildPixmap);
}

ImageModel::~ImageModel()
{
}

void ImageModel::Load(const ImageData& image) {
	image_ = image;
	
	pixmap_ = QPixmap();

	RebuildPixmap();

	set_loaded(!IsImageEmpty(image_));
}

void ImageModel::Clear() {
	image_ = {};
	pixmap_ = QPixmap();
	set_loaded(false);
}

#ifdef ANNO_USE_OPENCV
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
    case CV_16S: return GetChannelValuesForType<uint16_t, N>(m, x, y);
    case CV_32S: return GetChannelValuesForType<int32_t, N>(m, x, y);
    case CV_32F: return GetChannelValuesForType<float, N>(m, x, y);
    case CV_64F: return GetChannelValuesForType<double, N>(m, x, y);
    }
}
}

std::vector<float> ImageModel::GetBackgroundPixelValues(int x, int y) {
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

QImage ImageModel::ImageFromMat(const cv::Mat& image) {
    if (image.empty()) {
        return QImage();
    }

    cv::Mat image_to_convert;
    if (image.channels() == 1) {
        // Currently we need this image only to show, so convert opecnv to color, because this is easy
        cv::cvtColor(image, image_to_convert, cv::COLOR_GRAY2RGB);
    }
    else {
        image_to_convert = image.clone(); // TODO(ia):  inefficient.
    }

    switch (image_to_convert.type()) {
    case CV_32FC3:
        cv::pow(image_to_convert, get_exr_correction(), image_to_convert);
        image_to_convert.convertTo(image_to_convert, CV_8U, 255);
        break;

    case CV_16UC3:
        image_to_convert.convertTo(image_to_convert, CV_8U, 1. / 65535 * 255);
        break;

    case CV_8UC3:
        break;

    default:
        assert(!"Unsupported image type");
        return QImage();
    }

    QImage ref_image( // image that references the existing data
        image_to_convert.data,
        image_to_convert.cols,
        image_to_convert.rows,
        static_cast<int>(image_to_convert.step1()),
        QImage::Format_RGB888);

    return ref_image.rgbSwapped(); // swap R & B channels, make a deep copy. 
}

void ImageModel::RebuildPixmap() {
    auto result = image_.clone();

    if (get_grayscale() && result.channels() > 1) {
        cv::cvtColor(result, result, cv::COLOR_BGR2GRAY);
    }

    /*
    int brightness = get_brightness();
    int contrast = get_contrast();

    if ((brightness || contrast) && result.depth() <= CV_8S && result.channels() == 3) {
        ColorTransformer ct(brightness, contrast);
        for (int y = 0; y < result.rows; y++) {
            for (int x = 0; x < result.cols; x++) {
                result.at<cv::Vec3b>(y, x) = ct.Transform(result.at<cv::Vec3b>(y, x));
            }
        }
    }
    else if (brightness) { 
        // Fallback to old brightness algorithm if not 3-channels or not 8-bit/channel
		double beta;
		switch (result.depth()) {
		case CV_8U:
		case CV_8S:
			beta = brightness;
			break;

		default:
			beta = brightness * 0.01;
			break;
		}
		result.convertTo(result, result.type(), 1, beta);
	}
    */

    auto image = ImageFromMat(result);

    int brightness = get_brightness();
    int contrast = get_contrast();

    if (brightness || contrast) {
        ColorTransformer ct(brightness, contrast);
        for (int y = 0; y < image.height(); ++y) {
            auto p = image.scanLine(y);
            for (int x = 0; x < image.width(); ++x, p += 3) {
                ct.TransformB3(p);
            }
        }
    }

    pixmap_ = QPixmap::fromImage(image);

    emit pixmap_changed();
}
#else
std::vector<float> ImageModel::GetBackgroundPixelValues(int x, int y) {
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
}

void ImageModel::RebuildPixmap() {
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

    emit pixmap_changed();
}

#endif
