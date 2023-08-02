// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
//
// Anno Labeling Tool
// 2020-2023 (c) urobots GmbH, https://urobots.io

#include "LabelHandle.h"
#include "Label.h"

LabelHandle::LabelHandle(Label * parent)
: parent_(parent)
{
}

LabelHandle::LabelHandle(QPointF pos, Label *parent)
: pos_(pos)
, parent_(parent)
{
}

LabelHandle::~LabelHandle() {
}

void LabelHandle::OnPaint(const PaintInfo & pi) {		
	float n = 5.0 / pi.world_scale;

	auto old_brush = pi.painter->brush();
    pi.painter->setBrush(pi.is_selected ? Qt::white : Qt::transparent);
	
	QPen pen(Qt::red);
	pen.setWidth(0);    	
    if (!pi.is_selected) {
        pen.setStyle(Qt::DashLine);
    }
    pi.painter->setPen(pen);
	pi.painter->drawEllipse(QRectF(pos_.x() - n, pos_.y() - n, n * 2, n * 2));
	pi.painter->setBrush(old_brush);
}

void LabelHandle::SetPosition(QPointF pos, bool notify_parent) {

	auto delta = pos - pos_;

	pos_ = pos;

    if (notify_parent && parent_) {
        parent_->HandlePositionChanged(this, delta);
    }
}
