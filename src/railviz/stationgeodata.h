#ifndef STATIONGEODATA_H
#define STATIONGEODATA_H

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/box.hpp>

#include <boost/geometry/index/rtree.hpp>

#include <vector>

#include "../serialization/Schedule.h"
#include "../Edges.h"
#include "../Nodes.h"

namespace td {
namespace railviz {

namespace boostGEO = boost::geometry;

typedef boostGEO::model::point<float, 2, boostGEO::cs::cartesian> point;
typedef boostGEO::model::box<point> box;
typedef std::pair<box, std::pair<int, int>> edgesValue;

class StationGeoData
{
public:
    StationGeoData(Schedule& schedule);
private:
    Schedule& schedule;
    boostGEO::index::rtree< edgesValue, boostGEO::index::rstar<16> > edgesTree;
};

}}
#endif // STATIONGEODATA_H
