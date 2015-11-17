#include "motis/loader/gtfs/gtfs_parser.h"

#include "boost/filesystem.hpp"

#include "parser/csv.h"

#include "motis/loader/gtfs/files.h"
#include "motis/schedule-format/Schedule_generated.h"

#include "motis/loader/util.h"
#include "motis/loader/gtfs/agency.h"
#include "motis/loader/gtfs/stop.h"
#include "motis/loader/gtfs/route.h"
#include "motis/loader/gtfs/calendar.h"
#include "motis/loader/gtfs/calendar_date.h"
#include "motis/loader/gtfs/trip.h"
#include "motis/loader/gtfs/transfers.h"
#include "motis/loader/gtfs/stop_time.h"
#include "motis/loader/gtfs/services.h"

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
  auto agencies = read_agencies(loaded_file(root / AGENCY_FILE));
  auto stops = read_stops(loaded_file(root / STOPS_FILE));
  auto routes = read_routes(loaded_file(root / ROUTES_FILE), agencies);
  auto calendar = read_calendar(loaded_file(root / CALENDAR_FILE));
  auto dates = read_calendar_date(loaded_file(root / CALENDAR_DATES_FILE));
  auto services = traffic_days(calendar, dates);
  auto trips = read_trips(loaded_file(root / TRIPS_FILE), routes, services);
  auto transfers = read_transfers(loaded_file(root / TRANSFERS_FILE), stops);
  read_stop_times(loaded_file(root / STOPS_FILE), trips, stops);
}

}  // gtfs
}  // loader
}  // motis
