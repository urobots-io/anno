// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
//
// Anno Labeling Tool
// 2020-2021 (c) urobots GmbH, https://urobots.io

#include "RectLabel.h"

using namespace std;

struct HandleIndex {
    enum {
        top_left = 0,
        bottom_right,
        bottom_left,
        top_right,
        top,
        right,
        bottom,
        left,
        center,
        max
    };    
};

struct Anchor {
    enum {
        unknown = 0,
        start = 1,
        end
    };
};

RectLabel::RectLabel(const WorldInfo * wi)
    : creation_completed_(wi == nullptr) {
    QPointF position = wi ? wi->position : QPointF(0, 0);
    for (int i = 0; i < int(HandleIndex::max); ++i) {
        handles_.push_back(make_shared<LabelHandle>(position, this));
    }
    
    if (!creation_completed_) {
        width_.set(0);
        height_.set(0);
    }
    else {
        width_.set(default_dimension_);
        height_.set(default_dimension_);
        UpdateHandlesPositions();
    }
}

void RectLabel::InitStamp() {
    auto jw = category_->definition->stamp_parameters["width"];
    auto jh = category_->definition->stamp_parameters["height"];
        
    double width = jw.isDouble() ? jw.toDouble() : default_dimension_;
    double height = jh.isDouble() ? jh.toDouble() : default_dimension_;

    assert(category_);
    auto def = category_->definition;

    width_.set(def->GetSharedPropertyValue("width", width));
    height_.set(def->GetSharedPropertyValue("height", height));
}

LabelProperty *RectLabel::GetProperty(QString property_name) {
    if (property_name == "height") { return &height_; }
    if (property_name == "width") { return &width_; }
    return nullptr;
}

void RectLabel::ConnectSharedProperties(bool connect, bool inject_my_values) {
    assert(category_);
    auto def = category_->definition;
    if (connect) {
        def->ConnectProperty(width_, "width", inject_my_values);
        def->ConnectProperty(height_, "height", inject_my_values);
    }
    else {
        width_.Disconnect();
        height_.Disconnect();
    }
}

void RectLabel::UpdateSharedProperties() {
    if (width_.PullUpdate() + height_.PullUpdate()) {
        UpdateHandlesPositions();
    }
}

void RectLabel::CenterTo(QPointF position, double angle) {
    Q_UNUSED(angle)
    auto size2 = QPointF(width_.get(), height_.get()) / 2;
    handles_[0]->SetPosition(position - size2, false);
    handles_[1]->SetPosition(position + size2, false);
}

void RectLabel::OnPaint(const PaintInfo & pi, PaintExtraFunctions*) {
    pi.painter->setPen(GetOutlinePen(pi));
    auto p0 = handles_[0]->GetPosition();
    auto p1 = handles_[1]->GetPosition();
    pi.painter->drawRect(QRectF(p0, p1));
}

bool RectLabel::HitTest(const WorldInfo & wi) const {
    auto p0 = handles_[0]->GetPosition();
    auto p1 = handles_[1]->GetPosition();
    return QRectF(p0, p1).contains(wi.position);
}

double RectLabel::Area() const {
    return width_.get() * height_.get();
}

bool RectLabel::IsCreationFinished() const {
    return creation_completed_;
}

bool RectLabel::OnCreateMove(const WorldInfo & wi) {
    handles_[1]->SetPosition(wi.position);
    return true;
}

void RectLabel::OnCreateClick(const WorldInfo &, bool is_down) {
    if (is_down) {
        creation_completed_ = true;
    }
}

QTransform RectLabel::GetTransform(bool scale, bool rotate) {
    Q_UNUSED(rotate)
    auto h = handles_.front();
    auto pos = h->GetPosition();
    return QTransform().translate(pos.x(), pos.y()).scale(scale ? width_.get() : 1., scale ? height_.get() : 1.);
}

bool RectLabel::MoveBy(QPointF offset) {
    handles_[0]->SetPosition(handles_[0]->GetPosition() + offset, false);
    handles_[1]->SetPosition(handles_[1]->GetPosition() + offset, false);
    UpdateHandlesPositions();
    return true;
}

void RectLabel::HandlePositionChanged(LabelHandle *h, QPointF offset) {
    int index = -1;
    for (int i = 0; i < int(handles_.size()); ++i) {
        if (handles_[i].get() == h) {
            index = i;
            break;
        }
    }

    if (index == -1) {
        return;
    }

    auto start_pos = handles_[0]->GetPosition();
    auto end_pos = handles_[1]->GetPosition();

    switch (index) {
    case HandleIndex::center:
        handles_[0]->SetPosition(start_pos + offset, false);
        handles_[1]->SetPosition(end_pos + offset, false);
        break;

    case HandleIndex::left:
        handles_[0]->SetPosition(start_pos + QPointF(offset.x(), 0), false);
        width_.set(width_.get() - offset.x(), Anchor::end);
        break;

    case HandleIndex::right:
        width_.set(width_.get() + offset.x(), Anchor::start);
        break;

    case HandleIndex::top:
        handles_[0]->SetPosition(start_pos + QPointF(0, offset.y()), false);
        height_.set(height_.get() - offset.y(), Anchor::end);
        break;

    case HandleIndex::bottom:
        height_.set(height_.get() + offset.y(), Anchor::start);
        break;

    case HandleIndex::top_left:
        width_.set(width_.get() - offset.x(), Anchor::end);
        height_.set(height_.get() - offset.y(), Anchor::end);
        break;

        
    case HandleIndex::top_right:
        handles_[0]->SetPosition(start_pos + QPointF(0, offset.y()), false);
        width_.set(width_.get() + offset.x(), Anchor::start);
        height_.set(height_.get() - offset.y(), Anchor::end);
        break;

        
    case HandleIndex::bottom_left:
        handles_[0]->SetPosition(start_pos + QPointF(offset.x(), 0), false);
        width_.set(width_.get() - offset.x(), Anchor::end);
        height_.set(height_.get() + offset.y(), Anchor::start);
        break;
        
    case HandleIndex::bottom_right:
        width_.set(width_.get() + offset.x(), Anchor::start);
        height_.set(height_.get() + offset.y(), Anchor::start);
        break;        
    }

    UpdateHandlesPositions();
}

QStringList RectLabel::ToStringsList() {
    return QStringList() << ToString({ handles_[0], handles_[1] });
}

namespace {
int CalcFixedPosition(double a0, double a1, double b0, double b1) {
    auto d0 = fabs(a0 - b0);
    auto d1 = fabs(a1 - b1);
    if (d0 > d1) {
        return Anchor::end; 
    }
    else {
        return Anchor::start;
    }    
}
}

void RectLabel::FromStringsList(QStringList const &values) {
    auto h0 = handles_[0]->GetPosition();
    auto h1 = handles_[1]->GetPosition();

    DeleteHandles();
    FromString(values[0], handles_);

    while (handles_.size() < size_t(HandleIndex::max)) {
        handles_.push_back(make_shared<LabelHandle>(QPointF(0, 0), this));
    }    
    
    auto _h0 = handles_[0]->GetPosition();
    auto _h1 = handles_[1]->GetPosition();
    
    auto size = _h1 - _h0;
    width_.set(size.x(), CalcFixedPosition(h0.x(), h1.x(), _h0.x(), _h1.x()));
    height_.set(size.y(), CalcFixedPosition(h0.y(), h1.y(), _h0.y(), _h1.y()));

    creation_completed_ = true;

    UpdateHandlesPositions();
}

void RectLabel::UpdateHandlesPositions() {
    auto width = width_.get();
    auto height = height_.get();

    double x0 = handles_[HandleIndex::top_left]->GetPosition().x();
    double x1 = handles_[HandleIndex::bottom_right]->GetPosition().x();
    double y0 = handles_[HandleIndex::top_left]->GetPosition().y();
    double y1 = handles_[HandleIndex::bottom_right]->GetPosition().y();
    
    if (width_.IsShared()) {
        auto fixed_position = width_.iparam();
        if (fixed_position == Anchor::start) {
            x1 = x0 + width;
        }
        else if (fixed_position == Anchor::end) {
            x0 = x1 - width;
        }
        else {
            auto c = (x0 + x1) / 2;
            x0 = c - width / 2;
            x1 = c + width / 2;
        }
    }
    else {
        x1 = x0 + width;
    }

    if (height_.IsShared()) {
        auto fixed_position = height_.iparam();
        if (fixed_position == Anchor::start) {
            y1 = y0 + height;
        }
        else if (fixed_position == Anchor::end) {
            y0 = y1 - height;
        }
        else {
            auto c = (y0 + y1) / 2;
            y0 = c - height / 2;
            y1 = c + height / 2;
        }
    }
    else {
        y1 = y0 + height;
    }

    double xm = (x0 + x1) / 2;
    double ym = (y0 + y1) / 2;
    
    handles_[HandleIndex::top_left]->SetPosition(QPointF(x0, y0), false);
    handles_[HandleIndex::bottom_right]->SetPosition(QPointF(x1, y1), false);
    handles_[HandleIndex::bottom_left]->SetPosition(QPointF(x0, y1), false);
    handles_[HandleIndex::top_right]->SetPosition(QPointF(x1, y0), false);
    handles_[HandleIndex::top]->SetPosition(QPointF(xm, y0), false);
    handles_[HandleIndex::right]->SetPosition(QPointF(x1, ym), false);
    handles_[HandleIndex::bottom]->SetPosition(QPointF(xm, y1), false);
    handles_[HandleIndex::left]->SetPosition(QPointF(x0, ym), false);
    handles_[HandleIndex::center]->SetPosition(QPointF(xm, ym), false);
}

QString RectLabel::GetComment() {
    auto origin = handles_[0]->GetPosition();

    return QString("Rect(x y w h): %0 %1 %2 %3")
            .arg(origin.x())
            .arg(origin.y())
            .arg(width_.get())
            .arg(height_.get());
}
