#include "motis/loader/builders/hrd/direction_builder.h"

#include "parser/util.h"

#include "motis/loader/util.h"
#include "motis/loader/parsers/hrd/files.h"
#include "motis/loader/model/hrd/hrd_service.h"

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
    station_builder& sb, flatbuffers::FlatBufferBuilder& fbb) {
  if (directions.empty()) {
    return 0;
  } else {
    auto const direction_key = directions[0];
    return get_or_create(fbs_directions_, direction_key.first, [&]() {
      switch (direction_key.second) {
        case hrd_service::EVA_NUMBER: {
          return CreateDirection(
              fbb, sb.get_or_create_station(direction_key.first, fbb));
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
