#pragma once
#include <QLineF>

namespace geometry {

inline double WrapAngle(double src) {
    while (src > M_PI) src -= M_PI * 2;
    while (src < -M_PI) src += M_PI * 2;
    return src;
}

inline double Deg2Rad(double deg) {
    return deg * M_PI / 180.0;
}

inline double Rad2Deg(double rad) {
    return rad * 180.0 / M_PI;
}

#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
inline QLineF::IntersectionType Intersection(const QLineF & a, const QLineF& b, QPointF *point = nullptr) {
    return a.intersects(b, point);
}
#else
inline QLineF::IntersectType Intersection(const QLineF & a, const QLineF& b, QPointF *point = nullptr) {
    return a.intersect(b, point);
#endif
}

}
