#pragma once

#include <map>
#include <string>
#include <vector>

#include "parser/buffer.h"

#include "motis/path/prepare/schedule/bus_stop_positions.h"
#include "motis/path/prepare/schedule/station_sequences.h"

namespace motis {
namespace path {

struct schedule_wrapper {
  explicit schedule_wrapper(std::string const& schedule_path);

  std::vector<station_seq> load_station_sequences() const;

  std::map<std::string, std::vector<geo::latlng>> find_bus_stop_positions(
      std::string const& osm_file) const;

  parser::buffer schedule_buffer_;
};

}  // namespace path
}  // namespace motis
