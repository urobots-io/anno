// Anno Labeling Tool
// 2020-2026 (c) urobots GmbH, https://urobots.io/en/portfolio/anno/

#include "VideoSlider.h"
#include <QPainter>
#include <QStyle>
#include <QStyleOptionSlider>
#include <algorithm>

VideoSlider::VideoSlider(QWidget *parent)
    : QSlider(Qt::Horizontal, parent)
{
    setMinimumHeight(sizeHint().height() + band_height + 2);
}

void VideoSlider::SetMarkerCounts(std::vector<int> counts) {
    counts_ = std::move(counts);
    update();
}

void VideoSlider::paintEvent(QPaintEvent *event) {
    QSlider::paintEvent(event);

    int max_count = counts_.empty() ? 0 : *std::max_element(counts_.begin(), counts_.end());
    if (max_count <= 0) {
        return;
    }

    // same mapping from frames to pixels as the slider handle
    QStyleOptionSlider option;
    initStyleOption(&option);
    auto groove = style()->subControlRect(QStyle::CC_Slider, &option, QStyle::SC_SliderGroove, this);
    auto handle = style()->subControlRect(QStyle::CC_Slider, &option, QStyle::SC_SliderHandle, this);
    int span = std::max(1, groove.width() - handle.width());
    int x0 = groove.x() + handle.width() / 2;
    auto position = [&](int frame) {
        return QStyle::sliderPositionFromValue(minimum(), maximum(), frame, span);
    };

    // maximum count for every pixel, a frame covers at least one pixel
    std::vector<int> columns(span + 1, 0);
    int frames = int(counts_.size());
    for (int frame = 0; frame < frames; ++frame) {
        if (!counts_[frame]) {
            continue;
        }
        int a = position(frame);
        int b = frame + 1 < frames ? position(frame + 1) : span + 1;
        for (int x = a; x < std::max(a + 1, b) && x <= span; ++x) {
            columns[x] = std::max(columns[x], counts_[frame]);
        }
    }

    QPainter painter(this);
    const QColor base(0x0F, 0x6C, 0xBD);
    int y = height() - band_height;
    for (int x = 0; x <= span; ++x) {
        if (columns[x]) {
            QColor color = base;
            color.setAlpha(70 + 185 * columns[x] / max_count);
            painter.fillRect(QRect(x0 + x, y, 1, band_height), color);
        }
    }
}
