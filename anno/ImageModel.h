#pragma once
#include "ImageData.h"
#include "implement_q_property.h"
#include <QObject>
#include <QPixmap>

class ImageModel : public QObject
{
    Q_OBJECT

public:
    ImageModel(QObject *parent = nullptr);
    ~ImageModel();

	Q_PROPERTY(bool loaded READ get_loaded WRITE set_loaded NOTIFY loaded_changed);        
    Q_PROPERTY(bool grayscale READ get_grayscale WRITE set_grayscale NOTIFY grayscale_changed);

    void Load(const ImageData &);
	void Clear();

    virtual std::vector<float> GetBackgroundPixelValues(int x, int y) = 0;

    const QPixmap & GetPixmap() const { return pixmap_; }
    const ImageData & GetImageData() const { return image_; }

public slots:
    void RebuildPixmap();
	IMPLEMENT_Q_PROPERTY_WRITE(bool, loaded);

protected:
    virtual void RebuildPixmapInternal() = 0;

public slots:	
    IMPLEMENT_Q_PROPERTY_WRITE(bool, grayscale);
    
signals:
	void loaded_changed(bool);
    void grayscale_changed(bool);
    void pixmap_changed();

protected:
    /// grayscale source image
    bool grayscale_ = false;

    /// original background image
    ImageData image_;

    /// tranfromed background pixmap for display
    QPixmap pixmap_;

	/// image loaded (= valid)
	bool loaded_ = false;

public:    
    IMPLEMENT_Q_PROPERTY_READ(grayscale);
	IMPLEMENT_Q_PROPERTY_READ(loaded);    
};

class ImageModelQt : public ImageModel
{
    Q_OBJECT

public:
    ImageModelQt(QObject *parent = nullptr);
    ~ImageModelQt();

    Q_PROPERTY(int brightness READ get_brightness WRITE set_brightness NOTIFY brightness_changed);
    Q_PROPERTY(int contrast READ get_contrast WRITE set_contrast NOTIFY contrast_changed);

    std::vector<float> GetBackgroundPixelValues(int x, int y);

    QImage CropImage(QRect rect);

public slots:
    IMPLEMENT_Q_PROPERTY_WRITE(int, brightness);
    IMPLEMENT_Q_PROPERTY_WRITE(int, contrast);

signals:
    void contrast_changed(int);
    void brightness_changed(int);    

protected:
    void RebuildPixmapInternal();

private:
    /// modifier of the brightness (0 - no changes)
    int brightness_ = 0;

    /// modifier of the contrast (0 - no changes)
    int contrast_ = 0;

public:
    IMPLEMENT_Q_PROPERTY_READ(brightness);
    IMPLEMENT_Q_PROPERTY_READ(contrast);
};



#ifdef ANNO_USE_OPENCV

class ImageModelOpenCV : public ImageModel
{
    Q_OBJECT

public:
    ImageModelOpenCV(QObject *parent = nullptr);
    ~ImageModelOpenCV();

    Q_PROPERTY(double brightness READ get_brightness WRITE set_brightness NOTIFY brightness_changed);
    Q_PROPERTY(double contrast READ get_contrast WRITE set_contrast NOTIFY contrast_changed);
    Q_PROPERTY(double gamma READ get_gamma WRITE set_gamma NOTIFY gamma_changed);
    
    std::vector<float> GetBackgroundPixelValues(int x, int y);

    ImageData CropImage(QRect rect);

protected:
    void RebuildPixmapInternal();

public slots:
    IMPLEMENT_Q_PROPERTY_WRITE(double, brightness);
    IMPLEMENT_Q_PROPERTY_WRITE(double, contrast);
    IMPLEMENT_Q_PROPERTY_WRITE(double, gamma);

signals:
    void contrast_changed(double);
    void brightness_changed(double);
    void gamma_changed(double);

private:
    double brightness_ = 0;
    double contrast_ = 1;
    double gamma_ = 1;

public:
    IMPLEMENT_Q_PROPERTY_READ(brightness);
    IMPLEMENT_Q_PROPERTY_READ(contrast);
    IMPLEMENT_Q_PROPERTY_READ(gamma);
};

#endif

