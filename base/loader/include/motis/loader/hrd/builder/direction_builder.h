#pragma once

#include <cinttypes>
#include <map>
#include <vector>

#include "motis/schedule-format/Direction_generated.h"

#include "motis/loader/hrd/builder/station_builder.h"

namespace motis {
namespace loader {
namespace hrd {

struct direction_builder {
  explicit direction_builder(std::map<uint64_t, std::string>);

  flatbuffers::Offset<Direction> get_or_create_direction(
      std::vector<std::pair<uint64_t, int>> const&, station_builder&,
      flatbuffers::FlatBufferBuilder&);

  std::map<uint64_t, std::string> hrd_directions_;
  std::map<uint64_t, flatbuffers::Offset<Direction>> fbs_directions_;
};

}  // namespace hrd
}  // namespace loader
}  // namespace motis
