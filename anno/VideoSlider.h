#pragma once
#include <QSlider>
#include <vector>

/// Video frame slider. A band at the bottom shows where markers are present:
/// the more markers are active at a time, the stronger the color.
class VideoSlider : public QSlider {
public:
    VideoSlider(QWidget *parent = nullptr);

    /// number of active markers for every frame
    void SetMarkerCounts(std::vector<int> counts);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    static const int band_height = 5;
    std::vector<int> counts_;
};
