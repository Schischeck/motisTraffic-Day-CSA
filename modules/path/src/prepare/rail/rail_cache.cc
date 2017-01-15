#include "motis/path/prepare/rail/rail_cache.h"

#include <memory>

#include "parser/file.h"

#include "utl/to_vec.h"

#include "motis/path/fbs/RailWays_generated.h"

using namespace flatbuffers;
using namespace parser;

namespace motis {
namespace path {

std::vector<rail_way> load_rail_ways(std::string const& filename) {
  auto const buf = file{filename.c_str(), "r"}.content();

  return utl::to_vec(*GetRailWays(buf.buf_)->ways(), [](auto const& way) {
    return rail_way{way->from(), way->to(), utl::to_vec(*way->ids()),
                    utl::to_vec(*way->polyline(), [](auto const& pos) {
                      return geo::latlng{pos->lat(), pos->lng()};
                    })};
  });
}

void store_rail_ways(std::string const& filename,
                     std::vector<rail_way> const& ways) {
  FlatBufferBuilder fbb;
  auto const fbs_ways = utl::to_vec(ways, [&](auto const& way) {
    return CreateRailWay(fbb, way.from_, way.to_, fbb.CreateVector(way.ids_),
                         fbb.CreateVectorOfStructs(
                             utl::to_vec(way.polyline_, [](auto const& pos) {
                               return Position(pos.lat_, pos.lng_);
                             })));
  });

  fbb.Finish(CreateRailWays(fbb, fbb.CreateVector(fbs_ways)));
  file{filename.c_str(), "w+"}.write(fbb.GetBufferPointer(), fbb.GetSize());
}

}  // namespace path
}  // namespace motis
