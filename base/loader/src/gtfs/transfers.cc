#include "motis/loader/gtfs/transfers.h"

#include <algorithm>
#include <tuple>

#include "parser/csv.h"

#include "motis/loader/util.h"

using namespace parser;
using namespace flatbuffers;
using std::get;

namespace motis {
namespace loader {
namespace gtfs {

enum { from_stop_id, to_stop_id, transfer_type, min_transfer_time };

using gtfs_transfer = std::tuple<cstr, cstr, int, int>;
static const column_mapping<gtfs_transfer> columns = {
    {"from_stop_id", "to_stop_id", "transfer_type", "min_transfer_time"}};

std::map<stop_pair, transfer> read_transfers(loaded_file f,
                                             stop_map const& stops) {
  std::map<stop_pair, transfer> transfers;
  for (auto const& t : read<gtfs_transfer>(f.content(), columns)) {
    stop_pair key =
        std::make_pair(stops.at(get<from_stop_id>(t).to_str()).get(),
                       stops.at(get<to_stop_id>(t).to_str()).get());
    transfers.insert(std::make_pair(
        key, transfer(get<min_transfer_time>(t) / 60, get<transfer_type>(t))));
  }
  return transfers;
}

}  // namespace gtfs
}  // namespace loader
}  // namespace motis
