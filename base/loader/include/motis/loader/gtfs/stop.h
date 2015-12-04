#pragma once

#include <map>
#include <memory>
#include <string>

#include "motis/loader/loaded_file.h"

namespace motis {
namespace loader {
namespace gtfs {

struct stop {
  stop(std::string id, std::string name, double lat, double lng)
      : id_(std::move(id)), name_(std::move(name)), lat_(lat), lng_(lng) {}

  std::string id_;
  std::string name_;
  double lat_, lng_;
};

using stop_map = std::map<std::string, std::unique_ptr<stop>>;

stop_map read_stops(loaded_file);

}  // namespace gtfs
}  // namespace loader
}  // namespace motis
