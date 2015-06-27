#pragma once

#include "boost/geometry.hpp"
#include "boost/geometry/geometries/point.hpp"
#include "boost/geometry/geometries/box.hpp"

#include "boost/geometry/index/rtree.hpp"

#include "motis/core/schedule/edges.h"

namespace motis {
namespace railviz {
namespace geometry{

namespace bg = boost::geometry;

typedef bg::cs::spherical_equatorial<bg::degree> coordinate_system;
typedef bg::model::point<double, 2, coordinate_system> point;
typedef bg::model::box<point> box;

}
}
}
