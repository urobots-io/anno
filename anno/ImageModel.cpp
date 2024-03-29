// Anno Labeling Tool
// 2020-2024 (c) urobots GmbH, https://urobots.io/en/portfolio/anno/

#include "ImageModel.h"

ImageModel::ImageModel(QObject *parent)
    : QObject(parent)
{
    connect(this, &ImageModel::grayscale_changed, this, &ImageModel::RebuildPixmap);    
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

void ImageModel::RebuildPixmap() {
    RebuildPixmapInternal();    
    emit pixmap_changed();
}

