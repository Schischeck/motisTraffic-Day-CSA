#pragma once

#include "boost/geometry.hpp"
#include "boost/geometry/geometries/point.hpp"
#include "boost/geometry/geometries/box.hpp"

#include "boost/geometry/index/rtree.hpp"

#include "motis/core/schedule/edges.h"

namespace motis {
namespace railviz {

namespace bg = boost::geometry;

typedef bg::model::point<double, 2, bg::cs::cartesian> rtree_point;
typedef bg::model::box<rtree_point> rtree_box;
typedef std::pair<rtree_box, std::pair<unsigned int,unsigned int>> rtree_value;
typedef bg::index::rtree<rtree_value, bg::index::rstar<16>> rtree;

}
}
