#pragma once

#include <map>
#include <string>

#include "parser/cstr.h"

#include "motis/loader/loaded_file.h"
#include "motis/loader/parsers/gtfs/flat_map.h"

namespace motis {
namespace loader {
namespace gtfs {

struct transfer {
  enum {
    RECOMMENDED_TRANSFER,
    TIMED_TRANSFER,
    MIN_TRANSFER_TIME,
    NOT_POSSIBLE
  };

  transfer() = default;
  transfer(int minutes, int type) : minutes(minutes), type(type) {}

  int minutes;
  int type;
};

typedef std::pair<std::string, std::string> station_pair;
std::map<station_pair, transfer> read_transfers(loaded_file);

}  // namespace gtfs
}  // namespace loader
}  // namespace motis
