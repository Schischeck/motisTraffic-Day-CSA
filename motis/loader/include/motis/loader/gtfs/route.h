#pragma once

#include <string>
#include <memory>
#include <map>

#include "motis/loader/loaded_file.h"
#include "motis/loader/gtfs/agency.h"

namespace motis {
namespace loader {
namespace gtfs {

struct route {
  route(agency const* agency, std::string short_name, std::string long_name,
        int type)
      : agency_(agency),
        short_name_(std::move(short_name)),
        long_name_(std::move(long_name)),
        type_(type) {}

  agency const* agency_;
  std::string short_name_;
  std::string long_name_;
  int type_;
};

using route_map = std::map<std::string, std::unique_ptr<route>>;

route_map read_routes(loaded_file, agency_map const&);

}  // namespace gtfs
}  // namespace loader
}  // namespace motis
