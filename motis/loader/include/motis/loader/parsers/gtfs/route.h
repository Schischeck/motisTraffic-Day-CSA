#pragma once

#include <tuple>
#include <map>

#include "flatbuffers/flatbuffers.h"

#include "parser/cstr.h"

#include "motis/loader/loaded_file.h"

namespace motis {
namespace loader {
namespace gtfs {

struct route {
  std::string agency_id;
  std::string short_name, long_name;
  int type;
};

std::map<std::string, route> read_routes(loaded_file);

}  // namespace gtfs
}  // namespace loader
}  // namespace motis
