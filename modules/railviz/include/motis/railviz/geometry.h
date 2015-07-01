#pragma once

namespace motis {
namespace railviz {
namespace geometry {

struct point
{
    point() : lat(0), lng(0) {}
    point( double lat, double lng ) : lat(lat), lng(lng) {}
    double lat, lng;
};

struct box {
    box() {}
    box(point& p1, point& p2): p1(p1), p2(p2) {}
    point p1;
    point p2;
};

}
}
}
