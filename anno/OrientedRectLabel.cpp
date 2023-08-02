// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
//
// Anno Labeling Tool
// 2020-2023 (c) urobots GmbH, https://urobots.io

#include "OrientedRectLabel.h"
#include "geometry.h"
#include <QStringBuilder>
#include <QTextStream>

using namespace std;

#define ANGLE "angle"
#define WIDTH "width"
#define HEIGHT "height"

#define MAX_HANDLES 4

OrientedRectLabel::OrientedRectLabel(const WorldInfo * wi) {    
    QPointF position = wi ? wi->position : QPointF(0, 0);
    for (int i = 0; i < 1 + MAX_HANDLES; ++i) {
        handles_.push_back(make_shared<LabelHandle>(position, this));
    }

    if (wi) {
        state_ = State::creation_dimensions;
        angle_.set(wi->angle);
        width_.set(0);
        height_.set(0);
    }
    else {
        width_.set(default_dimension_);
        height_.set(default_dimension_);
        state_ = State::ready;
        UpdateHandlesPositions();
    }
}

QStringList OrientedRectLabel::GetPropertiesList() const {
    return { WIDTH, HEIGHT, ANGLE };
}

void OrientedRectLabel::InitStamp() {
    auto def = GetDefinition();
    if (!def) {
        return;
    }

    auto jw = def->stamp_parameters[WIDTH];
    auto jh = def->stamp_parameters[HEIGHT];
    
    double width = jw.isDouble() ? jw.toDouble() : default_dimension_;
    double height = jh.isDouble() ? jh.toDouble() : default_dimension_;
    
    width_.set(def->GetSharedPropertyValue(WIDTH, width));
    height_.set(def->GetSharedPropertyValue(HEIGHT, height));

    // do not take shared angle, because angle is defined by creation tool
    angle_.set(0);
}

LabelProperty *OrientedRectLabel::GetProperty(QString property_name) {
    if (property_name == ANGLE) { return &angle_; }
    if (property_name == HEIGHT) { return &height_; }
    if (property_name == WIDTH) { return &width_; }
    return nullptr;
}

void OrientedRectLabel::ConnectSharedProperties(bool connect, bool inject_my_values) {
    if (connect) {
        if (auto def = GetDefinition()) {
            def->ConnectProperty(width_, WIDTH, inject_my_values);
            def->ConnectProperty(height_, HEIGHT, inject_my_values);
            def->ConnectProperty(angle_, ANGLE, inject_my_values);
        }
    }
    else {
        width_.Disconnect();
        height_.Disconnect();
        angle_.Disconnect();
    }
}

void OrientedRectLabel::CenterTo(QPointF position, double angle) {
    angle_.set(angle);
    handles_[0]->SetPosition(position, false);
    UpdateHandlesPositions();
}

void OrientedRectLabel::UpdateHandlesPositions() {
	float cx2 = width_.get() / 2;
	float cy2 = height_.get() / 2;

	QPointF corners[] = {
		QPointF(cx2, cy2),
		QPointF(cx2, -cy2),
		QPointF(-cx2, -cy2),
		QPointF(-cx2, cy2)
	};

	QTransform t2;
	t2.rotate(geometry::Rad2Deg(angle_.get()));

	for (int i = 0; i < MAX_HANDLES; ++i) {
        corners[i] = handles_[0]->GetPosition() + t2.map(corners[i]);
		handles_[i + 1]->SetPosition(corners[i], false);
	}
}

void OrientedRectLabel::HandlePositionChanged(LabelHandle* h, const QPointF & offset) {
	if (handles_.size() < 1 + MAX_HANDLES) return;

	if (h == handles_[0].get()) {
		// change position of the box
        for (int i = 1; i < 1 + MAX_HANDLES; ++i) {
            handles_[i]->SetPosition(handles_[i]->GetPosition() + offset, false);
        }
	}
	else {
		// calculate new size 
		auto s2 = h->GetPosition() - handles_[0]->GetPosition();
		QTransform t;
		t.rotate(-geometry::Rad2Deg(angle_.get()));
        s2 = t.map(s2);
        width_.set(fabs(s2.x()) * 2);
        height_.set(fabs(s2.y()) * 2);
		UpdateHandlesPositions();
	}
}

void OrientedRectLabel::OnPaint(const PaintInfo & pi, PaintExtraFunctions*) {
	float cx2 = width_.get() / 2;	
	float cy2 = height_.get() / 2;
	float cxx = std::min(cx2, cy2) / 3;
	QPointF corners[] = {
		QPointF(cx2, cy2),
		QPointF(cx2, -cy2),
		QPointF(-cx2, -cy2),
		QPointF(-cx2, cy2),
		QPointF(-cx2, 0),
		QPointF(cx2, 0),

		QPointF(cx2 - cxx, cxx),
		QPointF(cx2 - cxx, -cxx)
	};

	QTransform t;
	t.rotate(geometry::Rad2Deg(angle_.get()));

	for (auto & p : corners) {
        p = handles_[0]->GetPosition() + t.map(p);
	}
	
	pi.painter->setPen(GetOutlinePen(pi));
	pi.painter->drawPolygon(corners, 4);	
	pi.painter->drawLine(corners[5], corners[4]);
	pi.painter->drawLine(corners[5], corners[6]);
	pi.painter->drawLine(corners[5], corners[7]);

	if (state_ == State::rotating) {
		QPen pen(Qt::white);
		pen.setWidth(0);
		pen.setStyle(Qt::DotLine);
		pi.painter->setPen(pen);
		pi.painter->drawLine(handles_[1 + rotating_index_]->GetPosition(), rotating_point_);
	}
}

bool OrientedRectLabel::HitTest(const WorldInfo & wi) const {
    QTransform t;
    t.rotate(geometry::Rad2Deg(-angle_.get()));

    auto p = t.map(wi.position - handles_[0]->GetPosition());

    float cx2 = width_.get() / 2;
    float cy2 = height_.get() / 2;
    return fabs(p.x()) <= cx2 && fabs(p.y()) <= cy2;
}

double OrientedRectLabel::Area() const {
    return width_.get() * height_.get();
}

bool OrientedRectLabel::IsCreationFinished() const {
	return state_ == State::ready;
}

bool OrientedRectLabel::OnCreateMove(const WorldInfo & wi) {
    if (state_ == State::creation_dimensions) {
        handles_[1]->SetPosition(wi.position);
    }
	else if (state_ == State::rotating) {
		// rotate
		QLineF line(handles_[0]->GetPosition(), wi.position);
		QLineF diagonal_line(QPointF(0, 0), QPointF(width_.get(), height_.get()));
		
		float diagonal_angle = diagonal_line.angle();
		float adds[] = {
			(diagonal_angle) * 2.0f,
			(90.0f - diagonal_angle) * 2.0f,
			(diagonal_angle) * 2.0f
		};
		for (int i = 0; i < rotating_index_; ++i) {
			diagonal_angle -= adds[i];
		}

		angle_.set(geometry::Deg2Rad(diagonal_angle - line.angle()));

		UpdateHandlesPositions();

		rotating_point_ = wi.position;
	}
	return true;
}

void OrientedRectLabel::OnCreateClick(const WorldInfo &, bool is_down) {
	if (is_down && state_ == State::creation_dimensions) state_ = State::ready;
	else if (!is_down && state_ == State::rotating) state_ = State::ready;
}

void OrientedRectLabel::CancelExtraAction() {
	state_ = State::ready;
}

bool OrientedRectLabel::StartExtraAction(const WorldInfo & wi, QStringList & data) {
	for (int i = 1; i < 1 + MAX_HANDLES; ++i) {
		auto p0 = handles_[i]->GetPosition();
		// calculate distance to p0
		if ((p0 - wi.position).manhattanLength() < 10 / wi.world_scale) {
            data = ToStringsList();
			state_ = State::rotating;
			rotating_index_ = i - 1;
			rotating_point_ = p0;
			return true;
		}
	}
	return false; 
}

QStringList OrientedRectLabel::ToStringsList() const {
	auto center = handles_[0]->GetPosition();

	QString string = 
	    QString::number(center.x()) % " " %
        QString::number(center.y()) % " " %
        QString::number(width_.get()) % " " %
        QString::number(height_.get()) % " " %
        QString::number(geometry::WrapAngle(angle_.get()));

	return QStringList() << string;
}

void OrientedRectLabel::FromStringsList(QStringList const & values) {
    DeleteHandles();

	float sx, sy, cx, cy, angle;
	QTextStream(&(QString&)values[0]) >> cx >> cy >> sx >> sy >> angle;

    angle_.set(angle);
    width_.set(sx);
    height_.set(sy);

	for (int i = 0; i < 1 + MAX_HANDLES; ++i) {
		auto point = make_shared<LabelHandle>(this);
		point->SetPosition(QPointF(cx, cy), false);
		handles_.push_back(point);
	}

    state_ = State::ready;

	UpdateHandlesPositions();
}

QTransform OrientedRectLabel::GetTransform(bool scale, bool rotate) {
    auto center = handles_[0]->GetPosition();   
    return QTransform()
        .translate(center.x(), center.y())
        .rotate(rotate ? geometry::Rad2Deg(angle_.get()) : 0)
        .scale(scale ? width_.get() * 0.5 : 1., scale ? height_.get() * 0.5 : 1.);
}

bool OrientedRectLabel::Rotate(double angle) {
    angle_.set(angle_.get() + angle);
    UpdateHandlesPositions();
    return true;
}

bool OrientedRectLabel::MoveBy(const QPointF & offset, bool use_own_cs) {    
    if (use_own_cs) {
        auto rotated_offset = QTransform().rotate(180.f / M_PI * angle_.get()).map(offset);
        handles_[0]->SetPosition(handles_[0]->GetPosition() + rotated_offset);
    }
    else {
        handles_[0]->SetPosition(handles_[0]->GetPosition() + offset);
    }	
	UpdateHandlesPositions();
	return true;
}

void OrientedRectLabel::UpdateSharedProperties(bool forced_update) {
    if ((angle_.PullUpdate() + width_.PullUpdate() + height_.PullUpdate()) > 0 || forced_update) {
        UpdateHandlesPositions();
    }
}
