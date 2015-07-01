#pragma once

#include <iostream>

namespace motis {
namespace railviz {
namespace geometry {

struct point
{
    point() : lat(0), lng(0) {}
    point(const point& p) : lat(p.lat), lng(p.lng) {}
    point( double lat, double lng ) : lat(lat), lng(lng) {}
    double lat, lng;
};

struct box {
    box() {}
    box(const box& b): p1(b.p1), p2(b.p2) {}
    box(point& p1, point& p2): p1(p1), p2(p2) {}
    point top_left() const {
        return point( std::min(p1.lat, p2.lat), std::min(p1.lng, p2.lng) );
    }
    point bottom_right() const {
        return point( std::max(p1.lat, p2.lat), std::max(p1.lng, p2.lng) );
    }
    point p1;
    point p2;
};

std::ostream& operator<< (std::ostream& stream, const point& p);
std::ostream& operator<< (std::ostream& stream, const box& box);

}
}
}
