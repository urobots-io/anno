#include "OrientedCircleLabel.h"
#include "geometry.h"
#include <QStringBuilder>
#include <QTextStream>

using namespace std;

OrientedCircleLabel::OrientedCircleLabel(const WorldInfo *wi)
: creation_completed_(wi == nullptr) {
    handles_.push_back(make_shared<LabelHandle>(this));
    handles_.push_back(make_shared<LabelHandle>(this));
    handles_.push_back(make_shared<LabelHandle>(this));
    
    QPointF position = wi ? wi->position : QPointF(0, 0);    
    if (creation_completed_) position += QPointF(default_dimension_, 0);       
    handles_.push_back(make_shared<LabelHandle>(position, this));

    if (wi) {
        handles_[0]->SetPosition(wi->position, false);
        angle_.set(wi->angle);
    }
}

void OrientedCircleLabel::InitStamp() {
    auto jrad = category_->definition->stamp_parameters["radius"];    
    double radius = jrad.isDouble() ? jrad.toDouble() : default_dimension_;

    auto jangle = category_->definition->stamp_parameters["angle"];
    double angle = jangle.isDouble() ? jangle.toDouble() : 0;

    assert(category_);
    auto def = category_->definition;
    radius_.set(def->GetSharedPropertyValue("radius", radius));
    radius_.set(def->GetSharedPropertyValue("angle", angle));

    handles_[0]->SetPosition(QPointF(0, 0), false);
    handles_[1]->SetPosition(QPointF(radius, 0), false);
}


void OrientedCircleLabel::OnNewDefinition() {
    UpdateHandlesPositions();
}

void OrientedCircleLabel::ConnectSharedProperties(bool connect, bool inject_my_values) {
    assert(category_);
    auto def = category_->definition;
    if (connect) {
        def->ConnectProperty(angle_, "angle", inject_my_values);
        def->ConnectProperty(radius_, "radius", inject_my_values);
    }
    else {
        angle_.Disconnect();
        radius_.Disconnect();
    }    
}

LabelProperty *OrientedCircleLabel::GetProperty(QString property_name) {
    if (property_name == "angle") { return &angle_; }
    if (property_name == "radius") { return &radius_; }
    return nullptr;
}

bool OrientedCircleLabel::HitTest(const WorldInfo & wi) const {
    auto p0 = handles_[0]->GetPosition();
    return QVector2D(wi.position - p0).length() <= radius_.get();
}

double OrientedCircleLabel::Area() const {
    return M_PI * radius_.get() * radius_.get();
}

bool OrientedCircleLabel::IsCreationFinished() const {
    return creation_completed_;
}

bool OrientedCircleLabel::OnCreateMove(const WorldInfo & wi) {
    handles_[3]->SetPosition(wi.position);
    return true;
}

void OrientedCircleLabel::OnCreateClick(const WorldInfo &, bool is_down) {
    if (is_down) creation_completed_ = true;
}

void OrientedCircleLabel::CenterTo(QPointF position, double angle) {
    angle_.set(angle);
    handles_[0]->SetPosition(position);
}

void OrientedCircleLabel::OnPaint(const PaintInfo & pi, PaintExtraFunctions*) {
	pi.painter->setPen(GetOutlinePen(pi));
    
    auto p0 = handles_[0]->GetPosition();

    pi.painter->drawLine(p0, handles_[1]->GetPosition());
    pi.painter->drawLine(p0, handles_[2]->GetPosition());
    
    auto radius = radius_.get();
    pi.painter->drawEllipse(p0, radius, radius);
}

QTransform OrientedCircleLabel::GetTransform(bool scale, bool rotate) {
    Q_UNUSED(scale)
    auto h = handles_.front();
    auto pos = h->GetPosition();
    return QTransform().translate(pos.x(), pos.y()).rotate(rotate ? geometry::Rad2Deg(angle_.get()) : 0);
}

QStringList OrientedCircleLabel::ToStringsList() {
    auto center = handles_[0]->GetPosition();
    QString string =
        QString::number(center.x()) % " " %
        QString::number(center.y()) % " " %
        QString::number(geometry::WrapAngle(angle_.get())) % "  " %
        QString::number(radius_.get());

    return QStringList() << string;
}

void OrientedCircleLabel::FromStringsList(QStringList const & values) {
    DeleteHandles();

    float cx, cy, angle, radius;
    QTextStream(&(QString&)values[0]) >> cx >> cy >> angle >> radius;

    for (int i = 0; i < 3; ++i)
        handles_.push_back(make_shared<LabelHandle>(QPointF(cx, cy), this));

    handles_.push_back(make_shared<LabelHandle>(QPointF(cx + radius, cy), this));

    angle_.set(angle);
    radius_.set(radius);

    UpdateHandlesPositions();

    creation_completed_ = true;
}

void OrientedCircleLabel::HandlePositionChanged(LabelHandle* h, QPointF offset) {
    Q_UNUSED(offset)
    if (handles_.size() < 4) 
        return;
    
    if (h == handles_[1].get()) {
        QLineF line(handles_[0]->GetPosition(), h->GetPosition());
		angle_.set(-geometry::Deg2Rad(line.angle()));
    }
    else if (h == handles_[2].get()) {
        QLineF line(handles_[0]->GetPosition(), h->GetPosition());
        angle_.set(geometry::Deg2Rad(-90 - line.angle()));
    }
    else if (h == handles_[3].get()) {
        auto center = handles_[0]->GetPosition();
        auto d = handles_[3]->GetPosition() - center;
        radius_.set(sqrt(d.x() * d.x() + d.y() * d.y()));
        return;
    }

    UpdateHandlesPositions();
}

void OrientedCircleLabel::UpdateHandlesPositions() {
    if (!category_ || handles_.empty())
        return;

    auto h = handles_.front();
    auto pos = h->GetPosition();

    QTransform t;
    t.rotate(geometry::Rad2Deg(angle_.get()));
    
    const auto &axis_array = category_->definition->axis_length;
    int axis_x = axis_array.size() > 0 ? axis_array[0] : axis_length;
    int axis_y = axis_array.size() > 1 ? axis_array[1] : axis_length;

    handles_[1]->SetEnabled(axis_x);
    if (axis_x) {
        handles_[1]->SetPosition(pos + t.map(QPointF(axis_x, 0)), false);
    }
    
    handles_[2]->SetEnabled(axis_y);
    if (axis_y) {
        handles_[2]->SetPosition(pos + t.map(QPointF(0, axis_y)), false);
    }   

    handles_[3]->SetPosition(pos + QPointF(radius_.get(), 0), false);
}

bool OrientedCircleLabel::Rotate(double angle) {
    angle_.set(angle_.get() + angle);
    UpdateHandlesPositions();
    return true;
}

bool OrientedCircleLabel::MoveBy(QPointF offset) {
	handles_[0]->SetPosition(handles_[0]->GetPosition() + offset);
	UpdateHandlesPositions();
	return true;
}

void OrientedCircleLabel::UpdateSharedProperties() {
    if (angle_.PullUpdate() + radius_.PullUpdate()) {
        UpdateHandlesPositions();
    }
}
