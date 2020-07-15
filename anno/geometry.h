#pragma once

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

}