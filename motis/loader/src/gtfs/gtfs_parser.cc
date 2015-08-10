#include "motis/loader/parsers/gtfs/gtfs_parser.h"

#include "boost/filesystem.hpp"

#include "parser/csv.h"

#include "motis/loader/parsers/gtfs/files.h"
#include "motis/schedule-format/Schedule_generated.h"

#include "motis/loader/util.h"
#include "motis/loader/parsers/gtfs/stop.h"

using namespace flatbuffers;
using namespace parser;
namespace fs = boost::filesystem;
using std::get;

namespace motis {
namespace loader {
namespace gtfs {

bool gtfs_parser::applicable(fs::path const& path) {
  auto const required_files = {AGENCY_FILE,   STOPS_FILE,      ROUTES_FILE,
                               TRIPS_FILE,    STOP_TIMES_FILE, TRANSFERS_FILE,
                               FEED_INFO_FILE};

  for (auto const& file_name : required_files) {
    if (!fs::is_regular_file(path / file_name)) {
      return false;
    }
  }

  if (!fs::is_regular_file(CALENDAR_FILE) &&
      !fs::is_regular_file(CALENDAR_DATES_FILE)) {
    return false;
  }

  return true;
}

void gtfs_parser::parse(fs::path const& path) {
  FlatBufferBuilder b;
  auto stations = read_stations(path, b);
}

}  // gtfs
}  // loader
}  // motis
