#include "motis/loader/parsers/gtfs/gtfs_parser.h"

#include "boost/filesystem.hpp"

#include "parser/csv.h"

#include "motis/loader/parsers/gtfs/files.h"
#include "motis/schedule-format/Schedule_generated.h"

#include "motis/loader/util.h"
#include "motis/loader/parsers/gtfs/stop.h"
#include "motis/loader/parsers/gtfs/route.h"

using namespace flatbuffers;
using namespace parser;
namespace fs = boost::filesystem;
using std::get;

namespace motis {
namespace loader {
namespace gtfs {

auto const required_files = {AGENCY_FILE,   STOPS_FILE,      ROUTES_FILE,
                             TRIPS_FILE,    STOP_TIMES_FILE, TRANSFERS_FILE,
                             FEED_INFO_FILE};

bool gtfs_parser::applicable(fs::path const& path) {
  for (auto const& file_name : required_files) {
    if (!fs::is_regular_file(path / file_name)) {
      return false;
    }
  }

  if (!fs::is_regular_file(path / CALENDAR_FILE) &&
      !fs::is_regular_file(path / CALENDAR_DATES_FILE)) {
    return false;
  }

  return true;
}

std::vector<std::string> gtfs_parser::missing_files(
    fs::path const& path) const {
  std::vector<std::string> files;
  if (!fs::is_directory(path)) {
    files.push_back((path).string().c_str());
  }

  std::copy_if(
      begin(required_files), end(required_files), std::back_inserter(files),
      [&](std::string const& f) { return !fs::is_regular_file(path / f); });

  if (!fs::is_regular_file(path / CALENDAR_FILE) &&
      !fs::is_regular_file(path / CALENDAR_DATES_FILE)) {
    files.push_back(CALENDAR_FILE);
    files.push_back(CALENDAR_DATES_FILE);
  }

  return files;
}

void gtfs_parser::parse(fs::path const& root, FlatBufferBuilder& b) {
  auto stations_buf = load_file(root / STOPS_FILE);
  auto routes_buf = load_file(root / ROUTES_FILE);

  auto stations = read_stations({STOPS_FILE, stations_buf}, b);
  auto routes = read_routes({ROUTES_FILE, routes_buf});
}

}  // gtfs
}  // loader
}  // motis
