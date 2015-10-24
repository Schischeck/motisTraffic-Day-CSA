#pragma once

#include <cinttypes>
#include <map>
#include <vector>

#include "motis/schedule-format/Direction_generated.h"

namespace motis {
namespace loader {
namespace hrd {

struct direction_builder {
  direction_builder(std::map<uint64_t, std::string>);

  flatbuffers::Offset<Direction> get_or_create_direction(
      std::vector<std::pair<uint64_t, int>> const&,
      std::map<int, flatbuffers::Offset<Station>> const&,
      flatbuffers::FlatBufferBuilder&);

  std::map<uint64_t, std::string> hrd_directions_;
  std::map<uint64_t, flatbuffers::Offset<Direction>> fbs_directions_;
};

}  // hrd
}  // loader
}  // motis
