#pragma once

#include <map>
#include <string>

#include "motis/loader/loaded_file.h"
#include "motis/loader/gtfs/trip.h"

namespace motis {
namespace loader {
namespace gtfs {

void read_stop_times(loaded_file const&, trip_map&, stop_map const&);

}  // namespace gtfs
}  // namespace loader
}  // namespace motis
