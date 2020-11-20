#include "PolylineLabel.h"
using namespace std;

PolylineLabel::PolylineLabel(const WorldInfo * wi) {
    if (wi) {
        state_ = State::creation;
        next_point_ = wi->position;
        handles_.push_back(make_shared<LabelHandle>(wi->position, this));
    }
    else {
        state_ = State::ready;
    }
}

PolylineLabel::~PolylineLabel() {
}

QStringList PolylineLabel::ToStringsList() {
    QStringList result;
    result << Label::ToString(handles_);
    return result;
}

void PolylineLabel::FromStringsList(QStringList const & value) {
    Label::FromStringsList(value);
    state_ = State::ready;
    UpdateInternalData();
}

void PolylineLabel::OnPaint(const PaintInfo & pi) {
    pi.painter->setPen(GetOutlinePen(pi));

    vector<QPointF> points;
    for (auto h : handles_) {
        points.push_back(h->GetPosition());
    }

    if (state_ == State::creation) points.push_back(next_point_);

    pi.painter->drawPolyline(&points[0], int(points.size()));
}

bool PolylineLabel::IsCreationFinished() const {
    return state_ == State::ready;
}

bool PolylineLabel::OnCreateMove(const WorldInfo & wi) {
    next_point_ = wi.position;
    return false;
}

bool PolylineLabel::MoveBy(QPointF offset) {
    for (auto h : handles_) {
        h->SetPosition(h->GetPosition() + offset, false);
    } 
    return true;
}

bool PolylineLabel::ForceCompleteCreation(const WorldInfo &) {
    state_ = State::ready;
    return handles_.size() > 1; 
}

void PolylineLabel::OnCreateClick(const WorldInfo & wi, bool is_down) {
    if (!is_down)
        return;
    
    handles_.push_back(make_shared<LabelHandle>(wi.position, this));    
}

void PolylineLabel::CancelExtraAction() {
    state_ = State::ready;   
}

bool PolylineLabel::HasExtraAction(const WorldInfo & wi, QString & description) {
    int index;
    auto result = DetectExtraAction(wi, index);
    switch (result) {
    default: break;
    case ExtraActionType::CreateHandle: description = "+ vertex"; break;
    case ExtraActionType::DeleteHandle: description = "- vertex"; break;
    }
    return result != ExtraActionType::Nothing;
}

bool PolylineLabel::StartExtraAction(const WorldInfo & wi, QStringList & data) {
    int index;    
    auto result = DetectExtraAction(wi, index);

    switch (result) {
    default:
        return false;

    case ExtraActionType::CreateHandle:
    {
        data = ToStringsList();
        auto point = make_shared<LabelHandle>(this);
        point->SetPosition(wi.position);
        handles_.insert(handles_.begin() + index + 1, point);
    }
    break;

    case ExtraActionType::DeleteHandle:
        data = ToStringsList();
        if (handles_.size() > 2)
        {
            auto point = handles_.at(index);
            point->ClearParent();            
            handles_.erase(std::find(handles_.begin(), handles_.end(), point));
        }
    break;   
    }

    return true;
}

PolylineLabel::ExtraActionType PolylineLabel::DetectExtraAction(const WorldInfo & wi, int & index) const {   
    // check handles
    for (size_t i = 0; i < handles_.size(); ++i) {
        auto p0 = handles_[i]->GetPosition();
        // calculate distance to p0
        if ((p0 - wi.position).manhattanLength() < 10 / wi.world_scale) {
            index = int(i);
            return ExtraActionType::DeleteHandle;
        }
    }

    // check edges
    for (size_t i = 0; i < handles_.size() - 1; ++i) {
        auto p0 = handles_[i]->GetPosition();
        auto p1 = handles_[i + 1]->GetPosition();

        // calculate distance to the edge
        QLineF edge(p0, p1);
        QLineF edge2(wi.position, wi.position + QPointF(7 / (2.f * wi.world_scale), 0));

        edge2.setAngle(edge.angle() + 90);
        edge2.setP1(edge2.p1() + edge2.p1() - edge2.p2());

        if (QLineF::BoundedIntersection == edge.intersects(edge2, nullptr)) {
            index = int(i);
            return ExtraActionType::CreateHandle;
        }
    }    

    return ExtraActionType::Nothing;
}

QTransform PolylineLabel::GetTransform(bool scale, bool rotate) {
    Q_UNUSED(scale)
    Q_UNUSED(rotate)
    auto h = handles_.front();
    auto pos = h->GetPosition();
    return QTransform().translate(pos.x(), pos.y());
}

void PolylineLabel::HandlePositionChanged(LabelHandle *, QPointF) {
    UpdateInternalData();
}

void PolylineLabel::UpdateInternalData() {
    if (handles_.size()) {
        auto p0 = handles_[0]->GetPosition();
        auto p1 = p0;
        for (size_t i = 1; i < handles_.size(); ++i) {
            auto p = handles_[i]->GetPosition();
            p0.setX(std::min(p0.x(), p.x()));
            p0.setY(std::min(p0.y(), p.y()));
            p1.setX(std::max(p1.x(), p.x()));
            p1.setY(std::max(p1.y(), p.y()));
        }
        aabb_ = QRectF(p0, p1);
    }
    else {
        aabb_ = QRectF();
    }
}

bool PolylineLabel::HitTest(const WorldInfo & wi) const {
    if (aabb_.contains(wi.position)) {
        int index;
        return DetectExtraAction(wi, index) != ExtraActionType::Nothing;
    }
    return false;
}
