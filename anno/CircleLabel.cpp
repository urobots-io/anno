// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
//
// Anno Labeling Tool
// 2020-2021 (c) urobots GmbH, https://urobots.io

#include "CircleLabel.h"
#include <QTextStream>
#include <QVector2D>

using namespace std;

#define RADIUS "radius"

CircleLabel::CircleLabel(const WorldInfo * wi)
: creation_completed_(wi == nullptr) {    
    QPointF position0 = wi ? wi->position : QPointF(0, 0);
    QPointF position1 = position0;
    if (creation_completed_) position1 += QPointF(default_dimension_, 0);
    handles_.push_back(make_shared<LabelHandle>(position0, this));
    handles_.push_back(make_shared<LabelHandle>(position1, this));    
}

QStringList CircleLabel::GetPropertiesList() const {
    return { RADIUS };
}

void CircleLabel::InitStamp() {  
    auto def = GetDefinition();
    if (!def) {
        return;
    }

    auto jval = def->stamp_parameters[RADIUS];
    double radius = jval.isDouble() ? jval.toDouble() : default_dimension_;

    radius_.set(def->GetSharedPropertyValue(RADIUS, radius));
    
    handles_[0]->SetPosition(QPointF(0, 0), false);
    handles_[1]->SetPosition(QPointF(radius, 0), false);
}

void CircleLabel::ConnectSharedProperties(bool connect, bool inject_my_values) {
    if (connect) {
        if (auto def = GetDefinition()) {
            def->ConnectProperty(radius_, RADIUS, inject_my_values);
        }
    }
    else {        
        radius_.Disconnect();
    }
}

void CircleLabel::UpdateSharedProperties() {
    if (radius_.PullUpdate()) {
        handles_[1]->SetPosition(handles_[0]->GetPosition() + QPointF(radius_.get(), 0), false);
    }
}

LabelProperty *CircleLabel::GetProperty(QString property_name) {
    if (property_name == RADIUS) { return &radius_; }
    return nullptr;
}

void CircleLabel::CenterTo(QPointF position, double angle) {
    Q_UNUSED(angle)
    handles_[0]->SetPosition(position);
}

void CircleLabel::OnPaint(const PaintInfo & pi, PaintExtraFunctions*) {
	pi.painter->setPen(GetOutlinePen(pi));

    auto p0 = handles_[0]->GetPosition();
	
	auto radius = radius_.get();	
	pi.painter->drawEllipse(p0, radius, radius);

    double s = std::min(radius / 5, 10.0 / pi.world_scale);
    pi.painter->drawLine(QPointF(p0.x() - s, p0.y()), QPointF(p0.x() + s, p0.y()));
    pi.painter->drawLine(QPointF(p0.x(), p0.y() - s), QPointF(p0.x(), p0.y() + s));
}

bool CircleLabel::HitTest(const WorldInfo & wi) const {
    auto p0 = handles_[0]->GetPosition();
    return QVector2D(wi.position - p0).length() <= radius_.get();
}

double CircleLabel::Area() const {
    return M_PI * radius_.get() * radius_.get();
}

bool CircleLabel::IsCreationFinished() const {
	return creation_completed_;
}

bool CircleLabel::OnCreateMove(const WorldInfo & wi) {
	handles_[1]->SetPosition(wi.position);
	return true;
}

void CircleLabel::OnCreateClick(const WorldInfo &, bool is_down) {
    if (is_down) creation_completed_ = true;
}

void CircleLabel::HandlePositionChanged(LabelHandle* h, const QPointF & offset) {
    if (handles_.size() != 2) {
        return;
    }

    if (h == handles_[0].get()) {
        // move circle
		handles_[1]->SetPosition(handles_[1]->GetPosition() + offset, false);
	}
    else {
        // change radius
        auto center = handles_[0]->GetPosition();
        auto d = handles_[1]->GetPosition() - center;
        radius_.set(sqrt(d.x() * d.x() + d.y() * d.y()));        
    }
}

QStringList CircleLabel::ToStringsList() const {
	auto center = handles_[0]->GetPosition();
	QStringList values;
	values << QString("%0 %1 ").arg(center.x()).arg(center.y());
	values << QString("%0").arg(radius_.get());
	return QStringList() << values.join(' ');
}

void CircleLabel::FromStringsList(QStringList const & values) {
    DeleteHandles();
	
	float cx, cy, radius;
	QTextStream(&(QString&)values[0]) >> cx >> cy >> radius;

    radius_.set(radius);

	auto center = make_shared<LabelHandle>(this);
	center->SetPosition(QPointF(cx, cy), false);
	handles_.push_back(center);

	auto handle1 = make_shared<LabelHandle>(this);
	handle1->SetPosition(QPointF(cx + radius, cy), false);
	handles_.push_back(handle1);

    creation_completed_ = true;
}

QTransform CircleLabel::GetTransform(bool scale, bool rotate) {
    Q_UNUSED(rotate)
    auto center = handles_[0]->GetPosition();
    auto radius = radius_.get();

    return QTransform()
        .translate(center.x(), center.y())
        .scale(scale ? radius : 1., scale ? radius : 1.);
}

bool CircleLabel::MoveBy(const QPointF & offset) {
	CenterTo(handles_[0]->GetPosition() + offset, 0);
	return true;
}
