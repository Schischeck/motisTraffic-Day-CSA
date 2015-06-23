#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/box.hpp>

#include <boost/geometry/index/rtree.hpp>

#include "motis/core/schedule/edges.h"

namespace motis {
namespace railviz {

namespace bg = boost::geometry;

typedef bg::model::point<double, 2, bg::cs::cartesian> RTreePoint;
typedef bg::model::box<RTreePoint> RTreeBox;
typedef std::pair<RTreeBox, std::pair<int,const motis::edge*>> RTreeValue;
typedef bg::index::rtree<RTreeValue, bg::index::rstar<16>> RTree;

}
}
