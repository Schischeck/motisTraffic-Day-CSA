#pragma once

#include <map>
#include <string>

#include "motis/loader/loaded_file.h"
#include "motis/schedule-format/Station_generated.h"

namespace motis {
namespace loader {
namespace gtfs {

struct trip {
  std::string route_id;
  std::string service_id;
  std::string headsign;
};

std::map<std::string, trip> read_trips(loaded_file);

}  // namespace gtfs
}  // namespace loader
}  // namespace motis
