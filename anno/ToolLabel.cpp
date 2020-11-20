#include "ToolLabel.h"
using namespace std;

ToolLabel::ToolLabel(const WorldInfo * wi) 
: PolylineLabel(wi) {   
}

ToolLabel::~ToolLabel() {
}

void ToolLabel::OnPaint(const PaintInfo & pi) {
    if (handles_.size() != 4 || state_ == State::creation) {
        PolylineLabel::OnPaint(pi);
        return;
    }
    

    auto p0 = handles_[0]->GetPosition();
    auto p1 = handles_[1]->GetPosition();
    auto p2 = handles_[2]->GetPosition();
    auto p3 = handles_[3]->GetPosition();

    QLineF line0(p0, p1);
    QLineF line1(p2, p3);
    
    

    QPointF point;
    auto intersection = line0.intersects(line1, &point);
    if (intersection == QLineF::NoIntersection) {
        return;
    }

    if (intersection == QLineF::UnboundedIntersection) {
        pi.painter->setPen(GetHelperPen(pi));
        
        pi.painter->drawLine(p0, point);
        pi.painter->drawLine(p1, point);
        pi.painter->drawLine(p2, point);
        pi.painter->drawLine(p3, point);
    }

    pi.painter->setPen(GetOutlinePen(pi));
    pi.painter->drawLine(line0);
    pi.painter->drawLine(line1);

    if (intersection != QLineF::NoIntersection) {
        double s = 1. / pi.world_scale;
        int font_size = 10 * s;
        auto font = pi.painter->font();
        if (font.pointSize() != font_size) {
            font.setBold(false);
            font.setPointSize(font_size);
            pi.painter->setFont(font);
        }        

        auto angle = line0.angleTo(line1);
        auto text = QString("%0").arg(angle);
        auto rectangle = QRectF(point.x() - 100, point.y() - 100, 200, 200);
        rectangle = pi.painter->boundingRect(rectangle, Qt::AlignCenter, text);

        auto d = rectangle.width() * 0.1;
        rectangle.adjust(-d, -d, d, d);

        pi.painter->setPen(GetHelperPen(pi));
        pi.painter->setBrush(QBrush(QColor::fromRgb(255, 255, 255, 220)));
        pi.painter->drawRect(rectangle);
        pi.painter->drawText(rectangle, Qt::AlignCenter, text);

        qreal r0 = 28;
        qreal r1 = 31;

        pi.painter->setBrush(Qt::NoBrush);
        pi.painter->drawEllipse(p2, r0, r0);
        pi.painter->drawEllipse(p3, r1, r1);
    }
}

QPen ToolLabel::GetHelperPen(const PaintInfo & pi) const {
    QColor color(category_->color);
    int line_width = -1;
    auto style = Qt::DotLine;
    QPen pen(style);
    pen.setColor(color);
    pen.setWidth(abs(line_width));
    pen.setCosmetic(line_width < 0);
    return pen;
}