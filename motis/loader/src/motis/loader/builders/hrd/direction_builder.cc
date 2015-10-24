#include "motis/loader/builders/hrd/direction_builder.h"

#include "parser/util.h"

#include "motis/loader/util.h"

namespace motis {
namespace loader {
namespace hrd {

using namespace flatbuffers;
using namespace parser;

direction_builder::direction_builder(
    std::map<uint64_t, std::string> hrd_directions)
    : hrd_directions_(std::move(hrd_directions)) {}

Offset<Direction> direction_builder::get_or_create_direction(
    std::vector<std::pair<uint64_t, int>> const& directions,
    std::map<int, flatbuffers::Offset<Station>> const& fbs_stations,
    FlatBufferBuilder& fbb) {
  if (directions.empty()) {
    return 0;
  } else {
    auto const direction_key = directions[0];
    return get_or_create(fbs_directions_, direction_key.first, [&]() {
      switch (direction_key.second) {
        case hrd_service::EVA_NUMBER: {
          auto const station_it = fbs_stations.find(direction_key.first);
          verify(station_it == end(fbs_stations), "missing station: %d",
                 direction_key.first);
          return CreateDirection(
              fbb, fbs_stations.find(direction_key.first)->second);
        }
        case hrd_service::DIRECTION_CODE: {
          auto it = fbs_directions_.find(direction_key.first);
          verify(it != end(fbs_directions_), "missing direction info: %lu",
                 direction_key.first);
          return CreateDirection(fbb, 0,
                                 to_fbs_string(fbb, it->second, ENCODING));
        }
        default: assert(false); return Offset<Direction>(0);
      }
    });
  }
}

}  // hrd
}  // loader
}  // motis
