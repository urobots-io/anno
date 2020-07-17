#pragma once
#include <QString>

enum class LabelType {	
	circle = 0,	
    oriented_point,
	oriented_rect,    
    point,
	polygon,
    polyline,
    rect,

    max_types
};

inline QString LabelTypeToString(LabelType label_type) {
	switch (label_type) {
	default:
	case LabelType::point: return "point";
    case LabelType::oriented_point: return "oriented_point";
	case LabelType::oriented_rect: return "oriented_rect";
	case LabelType::rect: return "rect";
	case LabelType::circle: return "circle";
	case LabelType::polygon: return "polygon";
    case LabelType::polyline: return "polyline";
	}
}

inline LabelType LabelTypeFromString(QString type_name) {
	if (type_name == "polygon") return LabelType::polygon;
    else if (type_name == "polyline") return LabelType::polyline;
	else if (type_name == "point") return LabelType::point;
    else if (type_name == "oriented_point") return LabelType::oriented_point;
	else if (type_name == "circle") return LabelType::circle;
	else if (type_name == "oriented_rect") return LabelType::oriented_rect;
	else if (type_name == "rect") return LabelType::rect;
	
	return LabelType::point;
}