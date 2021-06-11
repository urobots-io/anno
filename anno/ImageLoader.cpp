#include "ImageLoader.h"
#include "FilesystemInterface.h"
#ifdef ANNO_USE_OPENCV
#pragma warning(disable: 4996)
#include <ImfHeader.h>
#include <ImfInputFile.h>
#include <ImfIO.h>
#include <ImfAttribute.h>
#include <ImfStringAttribute.h>
#endif

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

#ifdef ANNO_USE_OPENCV
class MemStream : public Imf::IStream
{
    QByteArray buffer_;
    int pos_ = 0;

public:
    MemStream(QByteArray buffer) : Imf::IStream("memory"), buffer_(buffer), pos_(0) {}

    bool read(char c[/*n*/], int n) override {
        if (pos_ + n < buffer_.size()) {
            memcpy(&c[0], buffer_.data() + pos_, n);
            pos_ += n;
            return true;
        }
        else {
            return false;
        }
    }
    Imf::Int64 tellg() override { return pos_; }
    void seekg(Imf::Int64 pos) override { pos_ = pos; }
    void clear() override {}
};

#endif

void ImageLoader::run() {
    auto buffer = filesystem_->LoadFile(filename_);
    bool image_ok = true;

    if (buffer.size()) {
#ifdef ANNO_USE_OPENCV
        if (filename_.endsWith("exr")) {
            cv::Mat rawData(1, int(buffer.size()), CV_8UC1, (void*)buffer.data());
            image_ = cv::imdecode(rawData, cv::IMREAD_ANYCOLOR | cv::IMREAD_ANYDEPTH);
            if (image_.empty()) {
                error_text_ = tr("Cannot load file %0").arg(filename_);
                image_ok = false;
            }
        
            MemStream ms(buffer);
            Imf::InputFile input(ms);
            auto header = input.header();
            for (auto i = header.begin(); i != header.end(); ++i) {
                if (auto text = dynamic_cast<Imf::StringAttribute*>(&i.attribute())) {
                    properties_[QString::fromLatin1(i.name())] = QString::fromLatin1(text->value().c_str());
                }                                
            }
        }    
        else {
            QImage image;
            image.loadFromData(buffer);
            image = image.convertToFormat(QImage::Format_BGR888);
            cv::Mat(image.height(), image.width(), CV_8UC3, (cv::Scalar*)image.bits()).copyTo(image_);
            for (auto key : image.textKeys()) {
                properties_[key] = image.text(key);
            }
        }
#else
        QImage image;
        image.loadFromData(buffer);
        image_ = image;
        image_ok = !image_.isNull();

        for (auto key : image.textKeys()) {
            properties_[key] = image.text(key);
        }
#endif
    }

    if (converter_ && image_ok) {
        image_ = converter_->ConvertImage(image_, filename_, filesystem_, error_text_);
    }
}

