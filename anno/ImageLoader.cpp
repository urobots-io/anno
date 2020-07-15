#include "ImageLoader.h"
#include "FilesystemInterface.h"

using namespace std;

ImageLoader::ImageLoader(QObject *parent)
	: QThread(parent)
{
}

ImageLoader::~ImageLoader()
{
}

void ImageLoader::StartLoading(QString filename, std::shared_ptr<FilesystemInterface> filesystem, std::shared_ptr<ImageConverter> converter) {
	filename_ = filename;
    converter_ = converter;
    filesystem_ = filesystem;
	start();
}

void ImageLoader::run() {
    auto buffer = filesystem_->LoadFile(filename_);
    bool image_ok = true;

    if (buffer.size()) {
#ifdef ANNO_USE_OPENCV
        cv::Mat rawData(1, int(buffer.size()), CV_8UC1, (void*)buffer.data());
        image_ = cv::imdecode(rawData, cv::IMREAD_ANYCOLOR | cv::IMREAD_ANYDEPTH);
        if (image_.empty()) {
            error_text_ = tr("Cannot load file %0").arg(filename_);
            image_ok = false;
        }
#else
        QImage image;
        image.loadFromData(buffer);
        image_ = image;
        image_ok = !image_.isNull();
#endif
    }

    if (converter_ && image_ok) {
        image_ = converter_->ConvertImage(image_, filename_, filesystem_, error_text_);
    }
}

