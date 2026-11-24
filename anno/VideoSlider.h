#pragma once
#include <QSlider>
#include <vector>

class QPainter;
class QStyleOptionSlider;

/// Video frame slider. A band along the groove shows where markers are present:
/// the more markers are active at a time, the stronger the color.
class VideoSlider : public QSlider {
public:
    VideoSlider(QWidget *parent = nullptr);

    /// number of active markers for every frame
    void SetMarkerCounts(std::vector<int> counts);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void PaintMarkerBand(QPainter &painter, const QStyleOptionSlider &option);

    static const int band_height = 5;
    std::vector<int> counts_;
};
