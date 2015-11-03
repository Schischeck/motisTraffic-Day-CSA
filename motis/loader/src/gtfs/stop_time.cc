#include "motis/loader/parsers/gtfs/stop_time.h"

#include <tuple>
#include <algorithm>

#include "parser/csv.h"
#include "parser/arg_parser.h"

#include "motis/loader/util.h"

using std::get;
using namespace parser;

namespace motis {
namespace loader {
namespace gtfs {

using gtfs_stop_time = std::tuple<cstr, cstr, cstr, cstr, int, cstr, int, int>;
enum {
  trip_id,
  arrival_time,
  departure_time,
  stop_id,
  stop_sequence,
  stop_headsign,
  pickup_type,
  drop_off_type
};

static const column_mapping<gtfs_stop_time> stop_time_columns = {
    {"trip_id", "arrival_time", "departure_time", "stop_id", "stop_sequence",
     "stop_headsign", "pickup_type", "drop_off_type"}};

int hhmm_to_min(cstr s) {
  if (s.len == 0) {
    return -1;
  } else {
    int hours = 0;
    parse_arg(s, hours, 0);
    if (s) {
      ++s;
    } else {
      return -1;
    }

    int minutes = 0;
    parse_arg(s, minutes, 0);

    return hours * 60 + minutes;
  }
}

std::map<std::string, flat_map<stop_time>> read_stop_times(loaded_file file) {
  std::map<std::string, flat_map<stop_time>> stop_times;
  for (auto const& s : read<gtfs_stop_time>(file.content_, stop_time_columns)) {
    stop_times[get<trip_id>(s).to_str()].emplace(
        get<stop_sequence>(s),  // index
        get<stop_id>(s).to_str(),  // constructor arguments
        get<stop_headsign>(s).to_str(),  //
        hhmm_to_min(get<arrival_time>(s)), get<drop_off_type>(s) == 0,
        hhmm_to_min(get<departure_time>(s)), get<pickup_type>(s) == 0);
  }
  return stop_times;
}

}  // namespace gtfs
}  // namespace loader
}  // namespace motis
