#pragma once

#include <vector>

#include "boost/geometry/geometries/register/point.hpp"
#include "boost/geometry/geometries/register/linestring.hpp"

#include "motis/geo/latlng.h"

BOOST_GEOMETRY_REGISTER_POINT_2D(motis::geo::latlng, double,
                                 boost::geometry::cs::spherical_equatorial<boost::geometry::degree>,
                                 lng_, lat_);

BOOST_GEOMETRY_REGISTER_LINESTRING_TEMPLATED(std::vector);
