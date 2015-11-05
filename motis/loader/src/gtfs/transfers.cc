#include "motis/loader/gtfs/transfers.h"

#include <tuple>
#include <algorithm>

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
static const column_mapping<gtfs_transfer> transfer_columns = {
    {"from_stop_id", "to_stop_id", "transfer_type", "min_transfer_time"}};

std::map<station_pair, transfer> read_transfers(loaded_file f) {
  std::map<station_pair, transfer> transfers;
  for (auto const& t : read<gtfs_transfer>(f.content(), transfer_columns)) {
    transfer tr(get<min_transfer_time>(t) / 60, get<transfer_type>(t));
    transfers[std::make_pair(get<from_stop_id>(t).to_str(),
                             get<to_stop_id>(t).to_str())] = tr;
  }
  return transfers;
}

}  // namespace gtfs
}  // namespace loader
}  // namespace motis
