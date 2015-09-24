#pragma once

#include <tuple>

#include "parser/cstr.h"

namespace motis {
namespace loader {
namespace gtfs {

using stop_time = std::tuple<int,  // trip_id
                             parser::cstr,  // arrival_time
                             parser::cstr,  // departure_time
                             int,  // stop_id
                             int,  // stop_sequence
                             parser::cstr,  // stop_headsign
                             int,  // pickup_type
                             int,  // drop_off_type
                             int  // shape_dist_traveled
                             >;

static const std::array<parser::cstr,
                        std::tuple_size<stop_time>::value> stop_time_columns = {
    {"trip_id", "arrival_time", "departure_time", "stop_id", "stop_sequence",
     "stop_headsign", "pickup_type", "drop_off_type", "shape_dist_travele"}};

namespace stop_time_accessors {
enum {
  trip_id,
  arrival_time,
  departure_time,
  stop_id,
  stop_sequence,
  stop_headsign,
  pickup_type,
  drop_off_type,
  shape_dist_traveled
};
}  // namespace stop_time_accessors

}  // namespace gtfs
}  // namespace loader
}  // namespace motis
