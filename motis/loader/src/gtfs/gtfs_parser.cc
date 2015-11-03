#include "motis/loader/parsers/gtfs/gtfs_parser.h"

#include "boost/filesystem.hpp"

#include "parser/csv.h"

#include "motis/loader/parsers/gtfs/files.h"
#include "motis/schedule-format/Schedule_generated.h"

#include "motis/loader/util.h"
#include "motis/loader/parsers/gtfs/stop.h"
#include "motis/loader/parsers/gtfs/route.h"
#include "motis/loader/parsers/gtfs/calendar.h"
#include "motis/loader/parsers/gtfs/calendar_date.h"
#include "motis/loader/parsers/gtfs/trip.h"
#include "motis/loader/parsers/gtfs/transfers.h"
#include "motis/loader/parsers/gtfs/stop_time.h"
#include "motis/loader/parsers/gtfs/traffic_days.h"

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
  auto trips_buf = load_file(root / TRIPS_FILE);
  auto transfers_buf = load_file(root / TRANSFERS_FILE);
  auto calendar_buf = load_file(root / CALENDAR_FILE);
  auto calendar_dates_buf = load_file(root / CALENDAR_DATES_FILE);
  auto stop_times_buf = load_file(root / STOPS_FILE);

  auto stations = read_stations({STOPS_FILE, stations_buf}, b);
  auto routes = read_routes({ROUTES_FILE, routes_buf});
  auto trips = read_trips({TRIPS_FILE, trips_buf});
  auto transfers = read_transfers({TRANSFERS_FILE, transfers_buf});
  auto calendar = read_calendar({CALENDAR_FILE, calendar_buf});
  auto dates = read_calendar_date({CALENDAR_DATES_FILE, calendar_dates_buf});
  auto stop_times = read_stop_times({STOP_TIMES_FILE, stop_times_buf});
  auto services = traffic_days(calendar, dates);
}

}  // gtfs
}  // loader
}  // motis
