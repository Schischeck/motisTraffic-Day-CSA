#pragma once

namespace motis {
namespace railviz {
namespace geometry {

struct point
{
    double lat, lng;
};

struct box {
    point p1;
    point p2;
};

}
}
}
