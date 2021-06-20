#include "OrientedPointLabel.h"
#include "geometry.h"
#include <QStringBuilder>
#include <QTextStream>

using namespace std;

OrientedPointLabel::OrientedPointLabel(const WorldInfo *wi) {
    handles_.push_back(make_shared<LabelHandle>(this));
    handles_.push_back(make_shared<LabelHandle>(this));
    handles_.push_back(make_shared<LabelHandle>(this));

    if (wi) {
        handles_[0]->SetPosition(wi->position, false);
        angle_.set(wi->angle);
    }
}

void OrientedPointLabel::OnNewDefinition() {
    UpdateHandlesPositions();
}

void OrientedPointLabel::ConnectSharedProperties(bool connect, bool inject_my_values) {
    assert(category_);
    if (connect) {
        category_->definition->ConnectProperty(angle_, "angle", inject_my_values);
    }
    else {
        angle_.Disconnect();
    }    
}

LabelProperty *OrientedPointLabel::GetProperty(QString property_name) {
    if (property_name == "angle") { return &angle_; }
    return nullptr;
}

void OrientedPointLabel::CenterTo(QPointF position, double angle) {
    angle_.set(angle);
    handles_[0]->SetPosition(position);
}

void OrientedPointLabel::OnPaint(const PaintInfo & pi, PaintExtraFunctions*) {
	pi.painter->setPen(GetOutlinePen(pi));

    pi.painter->drawLine(handles_[0]->GetPosition(), handles_[1]->GetPosition());
    pi.painter->drawLine(handles_[0]->GetPosition(), handles_[2]->GetPosition());
}

QTransform OrientedPointLabel::GetTransform(bool scale, bool rotate) {
    Q_UNUSED(scale)
    auto h = handles_.front();
    auto pos = h->GetPosition();
    return QTransform().translate(pos.x(), pos.y()).rotate(rotate ? geometry::Rad2Deg(angle_.get()) : 0);
}

QStringList OrientedPointLabel::ToStringsList() {
    auto center = handles_[0]->GetPosition();
    QString string =
        QString::number(center.x()) % " " %
        QString::number(center.y()) % " " %
        QString::number(geometry::WrapAngle(angle_.get()));

    return QStringList() << string;
}

void OrientedPointLabel::FromStringsList(QStringList const & values) {
    DeleteHandles();

    float cx, cy, angle;
    QTextStream(&(QString&)values[0]) >> cx >> cy >> angle;

    for (int i = 0; i < 3; ++i)
        handles_.push_back(make_shared<LabelHandle>(QPointF(cx, cy), this));

    angle_.set(angle);

    UpdateHandlesPositions();
}

void OrientedPointLabel::HandlePositionChanged(LabelHandle* h, QPointF offset) {
    Q_UNUSED(offset)
    if (handles_.size() < 3) 
        return;
    
    if (h == handles_[1].get()) {
        QLineF line(handles_[0]->GetPosition(), h->GetPosition());
		angle_.set(-geometry::Deg2Rad(line.angle()));
    }
    else if (h == handles_[2].get()) {
        QLineF line(handles_[0]->GetPosition(), h->GetPosition());
        angle_.set(geometry::Deg2Rad(-90 - line.angle()));
    }

    UpdateHandlesPositions();
}

void OrientedPointLabel::UpdateHandlesPositions() {
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
}

bool OrientedPointLabel::Rotate(double angle) {
    angle_.set(angle_.get() + angle);
    UpdateHandlesPositions();
    return true;
}

bool OrientedPointLabel::MoveBy(QPointF offset) {
	handles_[0]->SetPosition(handles_[0]->GetPosition() + offset);
	UpdateHandlesPositions();
	return true;
}

void OrientedPointLabel::UpdateSharedProperties() {
    if (angle_.PullUpdate()) {
        UpdateHandlesPositions();
    }
}
