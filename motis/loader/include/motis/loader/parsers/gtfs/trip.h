#pragma once

#include <tuple>
#include <array>

#include "parser/cstr.h"

namespace motis {
namespace loader {
namespace gtfs {

using trip = std::tuple<int,  // route_id
                        int,  // service_id
                        int,  // trip_id
                        parser::cstr,  // trip_headsign
                        parser::cstr,  // trip_short_name
                        int,  // direction_id
                        int,  // block_id,
                        int  // shape_id
                        >;

static const std::array<parser::cstr, std::tuple_size<trip>::value>
    trip_columns = {{"route_id", "service_id", "trip_id", "trip_headsign",
                     "trip_short_name", "direction_id", "block_id",
                     "shape_id"}};

namespace trip_accessors {
enum {
  route_id,
  service_id,
  trip_id,
  trip_headsign,
  trip_short_name,
  direction_id,
  block_id,
  shape_id
};
}  // namespace trip_accessors

}  // namespace gtfs
}  // namespace loader
}  // namespace motis
