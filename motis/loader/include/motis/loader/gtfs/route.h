#pragma once

#include <string>
#include <tuple>
#include <map>

#include "parser/cstr.h"

#include "motis/loader/loaded_file.h"

namespace motis {
namespace loader {
namespace gtfs {

struct route {
  std::string agency_id, short_name, long_name;
  int type;
};

std::map<std::string, route> read_routes(loaded_file);

}  // namespace gtfs
}  // namespace loader
}  // namespace motis
