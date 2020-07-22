#pragma once
#include "CircleLabel.h"
#include "OrientedPointLabel.h"
#include "OrientedRectLabel.h"
#include "PointLabel.h"
#include "PolygonLabel.h"
#include "PolylineLabel.h"
#include "RectLabel.h"

struct LabelFactory {
    static std::shared_ptr<Label> CreateLabel(LabelType type, const WorldInfo *wi = nullptr) {        
        switch (type) {
        case LabelType::point: return std::make_shared<PointLabel>(wi);
        case LabelType::oriented_point: return std::make_shared<OrientedPointLabel>(wi);
        case LabelType::polygon: return std::make_shared<PolygonLabel>(wi);
        case LabelType::polyline: return std::make_shared<PolylineLabel>(wi);
        case LabelType::oriented_rect: return std::make_shared<OrientedRectLabel>(wi);
        case LabelType::rect: return std::make_shared<RectLabel>(wi);
        case LabelType::circle: return std::make_shared<CircleLabel>(wi);
        case LabelType::max_types:
            break;
        }
        return nullptr;
    }
};
