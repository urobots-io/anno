#include "PointLabel.h"
using namespace std;

PointLabel::PointLabel(const WorldInfo * wi) {
    handles_.push_back(make_shared<LabelHandle>(wi ? wi->position : QPointF(0, 0), this));}

void PointLabel::InitStamp() {
    handles_[0]->SetPosition(QPointF(0, 0), false);    
}

void PointLabel::CenterTo(QPointF position, double angle) {
    Q_UNUSED(angle)
    handles_[0]->SetPosition(position);
}

void PointLabel::OnPaint(const PaintInfo & pi, PaintExtraFunctions*) {
	pi.painter->setPen(GetOutlinePen(pi));

	auto h = handles_.front();
	auto pos = h->GetPosition();

	int s = 20;
	pi.painter->drawRect(QRectF(pos.x() - s/2, pos.y() - s/2, s, s));
}

QTransform PointLabel::GetTransform(bool scale, bool rotate) {
    Q_UNUSED(scale)
    Q_UNUSED(rotate)
    auto h = handles_.front();
    auto pos = h->GetPosition();
    return QTransform().translate(pos.x(), pos.y());
}

bool PointLabel::MoveBy(QPointF offset) {
	CenterTo(handles_[0]->GetPosition() + offset, 0);
	return true;
}
