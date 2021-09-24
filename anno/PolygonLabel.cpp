// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
//
// Anno Labeling Tool
// 2020-2021 (c) urobots GmbH, https://urobots.io

#include "PolygonLabel.h"
#include "geometry.h"
#include "triangulation/interface.h"

using namespace std;

PolygonLabel::PolygonLabel(const WorldInfo * wi) {
    if (wi) {
        state_ = State::creation;
        next_point_ = wi->position;
        contours_.push_back(make_shared<Handles>());
        OnCreateClick(*wi, true);
    }
    else {
        state_ = State::ready;
    }
}

PolygonLabel::~PolygonLabel() {
}

QStringList PolygonLabel::ToStringsList() const {
	QStringList result;
    for (auto c : contours_) {
        result << Label::ToString(*c);
    }
	return result;
}

void PolygonLabel::FromStringsList(QStringList const & list) {
    contours_.clear();
    DeleteHandles();

	for (auto s : list) {
		auto c = make_shared<Handles>();
		contours_.push_back(c);
		Label::FromString(s, *c);
        handles_.insert(handles_.end(), c->begin(), c->end());
	}

    Triangulate();
    state_ = State::ready;
}

void PolygonLabel::OnPaint(const PaintInfo & pi, PaintExtraFunctions*) {
	if (triangles_.size()) {
		pi.painter->setPen(Qt::transparent);
		
		auto color = category_->color;
		color.setAlpha(50);
		pi.painter->setBrush(QBrush(color));

		for (size_t i = 0; i < triangles_.size(); i += 3) {
			pi.painter->drawConvexPolygon(&triangles_[i], 3);
		}

		pi.painter->setBrush(Qt::transparent);        
	}
	
	pi.painter->setPen(GetOutlinePen(pi));

	switch (state_) {
	case State::ready:
		for (auto h : contours_) {
			RenderHandles(pi, *h, nullptr);
		}
		break;

	case State::creation:
	case State::cutting_hole:
		for (auto h : contours_) {
			RenderHandles(pi, *h,  (h == *contours_.rbegin()) ? &next_point_ : nullptr);
		}
		break;
	}	    
}

void PolygonLabel::RenderHandles(const PaintInfo & pi, const Handles & handles, QPointF *candidate) {
	vector<QPointF> points;
	for (auto h : handles) {
		points.push_back(h->GetPosition());
	}

	if (!candidate) {
		pi.painter->drawPolygon(&points[0], int(points.size()));
	}
	else {
		points.push_back(*candidate);
		pi.painter->drawPolyline(&points[0], int(points.size()));
	}
}

bool PolygonLabel::IsCreationFinished() const {
	return state_ == State::ready;
}

bool PolygonLabel::IsNearStartPoint(const WorldInfo & wi) const {
	auto h = *contours_.rbegin();
	if (h->size()) {
		auto start_pos = h->at(0)->GetPosition();
		if ((start_pos - wi.position).manhattanLength() < distance_to_finish_creation_ / wi.world_scale) { 
			return true;
		}
	}
	return false;
}

bool PolygonLabel::OnCreateMove(const WorldInfo & wi) {
	next_point_ = wi.position;
	return IsNearStartPoint(wi);
}

bool PolygonLabel::MoveBy(const QPointF & offset) {
    for (auto h : handles_) {
        h->SetPosition(h->GetPosition() + offset, false);
    }
    Triangulate();	
	return true;
}

void PolygonLabel::OnCreateClick(const WorldInfo & wi, bool is_down) {
	if (!is_down)
		return;

	if (IsNearStartPoint(wi)) {
		state_ = State::ready;
		Triangulate();
		return;		
	}

	auto point = make_shared<LabelHandle>(this);
	point->SetPosition(wi.position);
	handles_.push_back(point);

	(*contours_.rbegin())->push_back(point);
}

void PolygonLabel::CancelExtraAction() {
	state_ = State::ready; 
	
	// delete last contour
	auto contour = *contours_.rbegin();
	if (contour != contours_.front()) {
		for (auto p : *contour) {            
			DeleteHandle(p);
		}		
		contours_.pop_back();
	}

	Triangulate();
}

void PolygonLabel::SetComputeVisualisationData(bool value) {
    Label::SetComputeVisualisationData(value);
	Triangulate();
}

bool PolygonLabel::HitTest(const WorldInfo & wi) const {
    if (aabb_.contains(wi.position)) {
        for (size_t i = 0; i < triangles_.size(); i += 3) {
            QPolygonF poly;
            poly << triangles_[i] << triangles_[i + 1] << triangles_[i + 2];
            if (poly.containsPoint(wi.position, Qt::OddEvenFill)) {
                return true;
            }
        }
    }
	return false;
}

double PolygonLabel::Area() const {
    return area_;
}

bool PolygonLabel::HasExtraAction(const WorldInfo & wi, QString & description) {
	auto result = DetectExtraAction(wi);
	switch (result.type) {
	default: break;
	case ExtraActionType::CreateHandle: description = "+ vertex"; break;
	case ExtraActionType::DeleteHandle: description = "- vertex"; break;
	case ExtraActionType::CutHole: description = "+ hole"; break;
	}
	return result.type != ExtraActionType::Nothing;
}

bool PolygonLabel::StartExtraAction(const WorldInfo & wi, QStringList & data) {
	auto result = DetectExtraAction(wi);
	switch (result.type) {
	default: 
		return false;
	
	case ExtraActionType::CreateHandle:
	{
        data = ToStringsList();

		auto point = make_shared<LabelHandle>(this);
		point->SetPosition(wi.position);
		result.contour->insert(result.contour->begin() + result.index + 1, point);
		handles_.push_back(point);
	}
		break;
	
	case ExtraActionType::DeleteHandle: 
	{
        data = ToStringsList();

		if (result.contour != contours_.front() || result.contour->size() > 2) {
			auto point = result.contour->at(result.index);			
			result.contour->erase(result.contour->begin() + result.index);
            DeleteHandle(point);
			
			if (result.contour != contours_.front()) {
				if (result.contour->size() <= 2) {
					for (auto p : *result.contour) {						
						DeleteHandle(p);
					}
                    auto it = std::find(contours_.begin(), contours_.end(), result.contour);
                    if (it != contours_.end()) {
                        contours_.erase(it);
                    }
				}
			}
		}
	}
		break;
	
	case ExtraActionType::CutHole: 
	{
        data = ToStringsList();

		state_ = State::cutting_hole;

        
		auto point = make_shared<LabelHandle>(this);
		point->SetPosition(wi.position);
		handles_.push_back(point);

		auto contour = make_shared<Handles>();
		contours_.push_back(contour);
		contour->push_back(point);

		next_point_ = wi.position;
	}
		break;
	}

	Triangulate();

	return true;
}

PolygonLabel::ExtraAction PolygonLabel::DetectExtraAction(const WorldInfo & wi) {
    ExtraAction result;

	// check handles
	for (auto c : contours_) {
        for (int i = 0; i < int(c->size()); ++i) {
			auto p0 = c->at(i)->GetPosition();

			// calculate distance to p0
			if ((p0 - wi.position).manhattanLength() < 10 / wi.world_scale) {
				result.index = i;
				result.contour = c;
				result.type = ExtraActionType::DeleteHandle;
                return result;
			}
		}
	}

	// check edges
	for (auto c : contours_) {
        for (int i = 0; i < int(c->size()); ++i) {
			auto p0 = c->at(i)->GetPosition();
			auto p1 = c->at((i + 1) % c->size())->GetPosition();

			// calculate distance to the edge
			QLineF edge(p0, p1);
			QLineF edge2(wi.position, wi.position + QPointF(7 / (2.f * wi.world_scale), 0));

			edge2.setAngle(edge.angle() + 90);
			edge2.setP1(edge2.p1() + edge2.p1() - edge2.p2());

            if (QLineF::BoundedIntersection == geometry::Intersection(edge, edge2)) {
				result.index = i;
                result.contour = c;
                result.type = ExtraActionType::CreateHandle;
                return result;
			}
		}
	}

	// check area
	if (HitTest(wi)) {
		result.type = ExtraActionType::CutHole;
	}

	return result;
}

void PolygonLabel::HandlePositionChanged(LabelHandle *, const QPointF &) {
	Triangulate();
}

void PolygonLabel::Triangulate() {
    triangles_.clear();

	if (!compute_visualisation_data_) return;

	if (state_ != State::ready) return;

	std::vector<QLineF> all_lines;

	for (auto c : contours_) {
		if (c->size() < 3)
			return;

		std::vector<QLineF> lines;

		// verify order of handles
		double test = 0;
        for (size_t i = 0; i < c->size(); ++i) {
			auto p0 = c->at(i)->GetPosition();
			auto p1 = c->at((i + 1) % c->size())->GetPosition();
			test += (p1.x() - p0.x()) * (p1.y() + p0.y());
			lines.push_back(QLineF(p0, p1));
		}

		// make sure there are no intersections of segments in own contour
        for (size_t i = 0; i < lines.size(); ++i) {
			auto last = lines.size();
			if (!i) --last;
            for (size_t j = i + 2; j < last; ++j) {
                if (QLineF::BoundedIntersection == geometry::Intersection(lines[i], lines[j])) {
                    return;
                }				
			}
		}

		// make sure there are no intersections of segments with other contours
        for (size_t i = 0; i < lines.size(); ++i) {
            for (size_t j = 0; j < all_lines.size(); ++j) {
                if (QLineF::BoundedIntersection == geometry::Intersection(lines[i], all_lines[j])) {
                    return;
                }
			}
		}

		for (auto & l : lines) all_lines.push_back(l);

		if ((c == contours_.front() && test > 0) || 
			(c != contours_.front() && test < 0)) {
			std::reverse(c->begin(), c->end());
		}
	}


	// triangulate 
	double vertexes[500][2]; // TODO(ap): why 500? dynamic allocation?
	int triangles[500][3];
	int number_points[500];

	memset(triangles, 0, sizeof(triangles));
	memset(vertexes, 0, sizeof(vertexes));
	memset(number_points, 0, sizeof(number_points));

	int index = 1;
	int c_index = 0;
	for (auto c : contours_) {
		number_points[c_index++] = int(c->size());
		for (auto h : *c) {
			vertexes[index][0] = h->GetPosition().x();
			vertexes[index][1] = h->GetPosition().y();
			++index;
		}
	}

	triangulate_polygon(int(contours_.size()), number_points, vertexes, triangles);

	for (int i = 0; triangles[i][0]; ++i) {
		for (int j = 0; j < 3; ++j) {
			QPointF p;
			p.setX(vertexes[triangles[i][j]][0]);
			p.setY(vertexes[triangles[i][j]][1]);
			triangles_.push_back(p);
		}
	}

    area_ = 0;    

    if (triangles_.size()) {
        for (size_t i = 0; i < triangles_.size(); i += 3) {
            auto b = (triangles_[i + 1] - triangles_[i]);
            auto c = (triangles_[i + 2] - triangles_[i]);
            area_ += (b.x() * c.y() - c.x() * b.y()) * 0.5;
            
        }

        auto p0 = triangles_[0];
        auto p1 = triangles_[0];
        for (auto p :triangles_) {
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

QTransform PolygonLabel::GetTransform(bool scale, bool rotate) {
    Q_UNUSED(scale)
    Q_UNUSED(rotate)
    auto h = handles_.front();
    auto pos = h->GetPosition();
    return QTransform().translate(pos.x(), pos.y());
}

