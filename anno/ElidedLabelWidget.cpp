// Anno Labeling Tool
// 2020-2024 (c) urobots GmbH, https://urobots.io/en/portfolio/anno/

#include "ElidedLabelWidget.h"
#include <QPainter>
#include <QSizePolicy>
#include <QTextLayout>

ElidedLabelWidget::ElidedLabelWidget(QWidget *parent)
: QWidget(parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
}

void ElidedLabelWidget::setText(const QString &newText) {
    content_ = newText;
    update();
}

void ElidedLabelWidget::setWidthHint(int value) {
    widthHint_ = value;
    updateGeometry();
}

void ElidedLabelWidget::setElideMode(Qt::TextElideMode value) {
    elideMode_ = value;
    update();
}

void ElidedLabelWidget::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);

    QPainter painter(this);
    QFontMetrics fontMetrics = painter.fontMetrics();

    int lineSpacing = fontMetrics.lineSpacing();
    
    QString line = fontMetrics.elidedText(content_, elideMode_, width());
    painter.drawText(QPoint(0, fontMetrics.ascent() + (height() - lineSpacing) / 2), line);
}
