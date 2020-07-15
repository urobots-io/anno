#pragma once
#include <string>
#include <math.h>

struct SharedPropertyDefinition {
    std::string name;
    double a = 1.0;
    double b = 0.0;

    bool IsIdentity() const {
        const double eps = 0.0000001;
        return fabs(a - 1.0) < eps && fabs(b) < eps;
    }

    double FromDatabaseValue(double x) {
        return a * x + b;
    }

    double ToDatabaseValue(double y) {
        return (y - b) / a;
    }
};
