#pragma once

#include <map>
#include <string>

#include "parser/cstr.h"

#include "motis/loader/loaded_file.h"
#include "motis/loader/gtfs/stop.h"

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
  transfer(int minutes, int type) : minutes_(minutes), type_(type) {}

  int minutes_;
  int type_;
};

typedef std::pair<stop const*, stop const*> stop_pair;
std::map<stop_pair, transfer> read_transfers(loaded_file, stop_map const&);

}  // namespace gtfs
}  // namespace loader
}  // namespace motis
