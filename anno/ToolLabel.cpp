#include "ToolLabel.h"
#include "geometry.h"
using namespace std;

namespace {
float FixAngle(float angle) {
    if (angle < 180.f) {
        return angle;
    }
    else {
        return 360.f - angle;
    }
}
}

ToolLabel::ToolLabel(const WorldInfo * wi) 
: PolylineLabel(wi) {   
}

ToolLabel::~ToolLabel() {
}

void ToolLabel::OnPaint(const PaintInfo & pi, PaintExtraFunctions* pf) {    
    switch (handles_.size()){
    default:
        PolylineLabel::OnPaint(pi, pf);
        return;

    case 2:
        Paint2(pi, pf);
        break;

    case 3:
        Paint3(pi, pf);
        break;

    case 4:
        Paint4(pi, pf);
        break;
    }
    
    if (state_ == State::creation) {
        vector<QPointF> points = {
            handles_[handles_.size() - 1]->GetPosition(),
            next_point_
        };

        pi.painter->setPen(GetHelperPen(pi));
        pi.painter->drawPolyline(&points[0], int(points.size()));
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

void ToolLabel::Paint2(const PaintInfo & pi, PaintExtraFunctions* pf) {
    auto p0 = handles_[0]->GetPosition();
    auto p1 = handles_[1]->GetPosition();

    QLineF line0(p0, p1);
    
    pi.painter->setPen(GetOutlinePen(pi));
    pi.painter->drawLine(line0);
    

    auto v = (p1 - p0);
    auto distance = sqrt(v.x() * v.x() + v.y() * v.y());
    pf->AddHint(QString("%0 px").arg(distance), (p0 + p1) * 0.5, category_->color);
}

void ToolLabel::Paint3(const PaintInfo & pi, PaintExtraFunctions* pf) {
    auto p0 = handles_[0]->GetPosition();
    auto p1 = handles_[1]->GetPosition();
    auto p2 = handles_[2]->GetPosition();

    QLineF line0(p0, p1);
    QLineF line1(p2, p1);

    pi.painter->setPen(GetOutlinePen(pi));
    pi.painter->drawLine(line0);
    pi.painter->drawLine(line1);

    auto angle = FixAngle(line0.angleTo(line1));
    pf->AddHint(QString::fromWCharArray(L"%0°").arg(angle), p1, category_->color);
}

void ToolLabel::Paint4(const PaintInfo & pi, PaintExtraFunctions* pf) {
    auto p0 = handles_[0]->GetPosition();
    auto p1 = handles_[1]->GetPosition();
    auto p2 = handles_[2]->GetPosition();
    auto p3 = handles_[3]->GetPosition();

    QLineF line0(p0, p1);
    QLineF line1(p2, p3);

    QPointF point;
    auto intersection = geometry::Intersection(line0, line1, &point);
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
        auto angle = FixAngle(line0.angleTo(line1));
        pf->AddHint(QString::fromWCharArray(L"%0°").arg(angle), point, category_->color);
    }
}

