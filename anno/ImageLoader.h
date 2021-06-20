#pragma once
#include "ImageConverter.h"
#include <QThread>
#include <memory>

struct FilesystemInterface;

class ImageLoader : public QThread
{
	Q_OBJECT

public:
	ImageLoader(QObject *parent);
    ~ImageLoader() override;

	void StartLoading(QString filename, std::shared_ptr<FilesystemInterface> filesystem, std::shared_ptr<ImageConverter> converter);
    QString GetErrorText() const { return error_text_; }
    QString GetFilename() const { return filename_; }
	
    ImageData GetLoadedImage() const { return image_; }

    const ImagePropertiesList& GetProperties() const { return properties_; }

private:
	void run() override;

private:
	QString filename_;
    std::shared_ptr<ImageConverter> converter_;
    ImageData image_;
    QString error_text_;
    ImagePropertiesList properties_;
    std::shared_ptr<FilesystemInterface> filesystem_;
};
