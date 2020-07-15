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
    Q_PROPERTY(int brightness READ get_brightness WRITE set_brightness NOTIFY brightness_changed);
    Q_PROPERTY(int contrast READ get_contrast WRITE set_contrast NOTIFY contrast_changed);
    Q_PROPERTY(double exr_correction READ get_exr_correction WRITE set_exr_correction NOTIFY exr_correction_changed);

    void Load(const ImageData &);
	void Clear();

    std::vector<float> GetBackgroundPixelValues(int x, int y);

    const QPixmap & GetPixmap() const { return pixmap_; }

private slots:
    void RebuildPixmap();
	IMPLEMENT_Q_PROPERTY_WRITE(bool, loaded);

public slots:	
    IMPLEMENT_Q_PROPERTY_WRITE(bool, grayscale);
    IMPLEMENT_Q_PROPERTY_WRITE(int, brightness);
    IMPLEMENT_Q_PROPERTY_WRITE(int, contrast);
    IMPLEMENT_Q_PROPERTY_WRITE(double, exr_correction);

signals:
	void loaded_changed(bool);
    void grayscale_changed(bool);
    void contrast_changed(int);
    void brightness_changed(int);
    void exr_correction_changed(int);
    void pixmap_changed();

private:
#ifdef ANNO_USE_OPENCV
    QImage ImageFromMat(const cv::Mat& mat);
	static unsigned char ClampToByte(int value);
	static unsigned char GetIntensityByte(cv::Vec3b value);
#endif

private:
    /// grayscale source image
    bool grayscale_ = false;

    /// modifier of the brightness (0 - no changes)
    int brightness_ = 0;

    /// modifier of the contrast (0 - no changes)
    int contrast_ = 0;

    /// exr image values will be pow'ed on this number
    double exr_correction_ = 0.4;

    /// original background image
    ImageData image_;

    /// tranfromed background pixmap for display
    QPixmap pixmap_;

	/// image loaded (= valid)
	bool loaded_ = false;

public:    
    IMPLEMENT_Q_PROPERTY_READ(brightness);
    IMPLEMENT_Q_PROPERTY_READ(contrast);
	IMPLEMENT_Q_PROPERTY_READ(grayscale);
	IMPLEMENT_Q_PROPERTY_READ(loaded);
    IMPLEMENT_Q_PROPERTY_READ(exr_correction);
};
