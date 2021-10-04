#pragma once
#include <string>
#include <math.h>

struct SharedPropertyDefinition {
    std::string name;
    double a = 1.0;
    double b = 0.0;
    
    inline static double eps() {
        return 0.0000001;
    }

    bool IsIdentity() const {        
        return fabs(a - 1.0) < eps() && fabs(b) < eps();
    }

    double FromDatabaseValue(double x) {
        return a * x + b;
    }

    double ToDatabaseValue(double y) {
        if (fabs(a) < eps()) {
            return 1.0;
        }

        return (y - b) / a;
    }
};
