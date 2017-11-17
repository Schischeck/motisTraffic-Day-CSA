#include "motis/loader/gtfs/gtfs_parser.h"

#include <numeric>

#include "boost/filesystem.hpp"

#include "utl/get_or_create.h"

#include "motis/core/common/date_time_util.h"
#include "motis/loader/gtfs/agency.h"
#include "motis/loader/gtfs/calendar.h"
#include "motis/loader/gtfs/calendar_date.h"
#include "motis/loader/gtfs/files.h"
#include "motis/loader/gtfs/route.h"
#include "motis/loader/gtfs/services.h"
#include "motis/loader/gtfs/stop.h"
#include "motis/loader/gtfs/stop_time.h"
#include "motis/loader/gtfs/transfers.h"
#include "motis/loader/gtfs/trip.h"
#include "motis/loader/util.h"
#include "motis/schedule-format/Schedule_generated.h"

using namespace flatbuffers64;
namespace fs = boost::filesystem;
using std::get;

namespace motis {
namespace loader {
namespace gtfs {

template <std::size_t N>
struct get_n {
  template <typename T>
  auto operator()(T&& t) const -> decltype(std::get<N>(std::forward<T>(t))) {
    return std::get<N>(std::forward<T>(t));
  }
};

auto const required_files = {AGENCY_FILE, STOPS_FILE,      ROUTES_FILE,
                             TRIPS_FILE,  STOP_TIMES_FILE, TRANSFERS_FILE};

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
    files.emplace_back(path.string());
  }

  std::copy_if(
      begin(required_files), end(required_files), std::back_inserter(files),
      [&](std::string const& f) { return !fs::is_regular_file(path / f); });

  if (!fs::is_regular_file(path / CALENDAR_FILE) &&
      !fs::is_regular_file(path / CALENDAR_DATES_FILE)) {
    files.emplace_back(CALENDAR_FILE);
    files.emplace_back(CALENDAR_DATES_FILE);
  }

  return files;
}

void gtfs_parser::parse(fs::path const& root, FlatBufferBuilder& fbb) {
  auto const load = [&](char const* file) { return loaded_file(root / file); };
  auto const agencies = read_agencies(load(AGENCY_FILE));
  auto const stops = read_stops(load(STOPS_FILE));
  auto const routes = read_routes(load(ROUTES_FILE), agencies);
  auto const calendar = read_calendar(load(CALENDAR_FILE));
  auto const dates = read_calendar_date(load(CALENDAR_DATES_FILE));
  auto const services = traffic_days(calendar, dates);
  auto const transfers = read_transfers(load(TRANSFERS_FILE), stops);
  auto trips = read_trips(load(TRIPS_FILE), routes, services);
  read_stop_times(load(STOP_TIMES_FILE), trips, stops);

  std::map<int, Offset<Category>> fbs_categories;
  std::map<agency const*, Offset<Provider>> fbs_providers;
  std::map<std::string, Offset<String>> fbs_strings;
  std::map<stop const*, Offset<Station>> fbs_stations;
  std::map<trip::stop_seq, Offset<Route>> fbs_routes;
  std::vector<Offset<Service>> fbs_services;

  auto get_or_create_stop = [&](stop const* s) {
    return utl::get_or_create(fbs_stations, s, [&]() {
      return CreateStation(fbb, fbb.CreateString(s->id_),
                           fbb.CreateString(s->name_), s->lat_, s->lng_, 5,
                           fbb.CreateVector(std::vector<Offset<String>>()));
    });
  };

  auto get_or_create_category = [&](int type) {
    return utl::get_or_create(fbs_categories, type, [&]() {
      return CreateCategory(fbb, fbb.CreateString(route::s_types_.at(type)), 2);
    });
  };

  auto get_or_create_provider = [&](agency const* a) {
    return utl::get_or_create(fbs_providers, a, [&]() {
      return CreateProvider(fbb, fbb.CreateString(a->id_),
                            fbb.CreateString(a->name_), fbb.CreateString(""));
    });
  };

  auto get_or_create_str = [&](std::string const& s) {
    return utl::get_or_create(fbs_strings, s,
                              [&]() { return fbb.CreateString(s); });
  };

  auto const interval =
      Interval{static_cast<uint64_t>(to_unix_time(services.first_day_)),
               static_cast<uint64_t>(to_unix_time(services.last_day_))};
  auto const output_services = fbb.CreateVector(utl::to_vec(
      begin(trips), end(trips),
      [&](std::pair<std::string const, std::unique_ptr<trip>> const& entry) {
        auto const& t = entry.second;
        auto const stop_seq = t->stops();
        return CreateService(
            fbb,
            utl::get_or_create(
                fbs_routes, stop_seq,
                [&]() {
                  return CreateRoute(
                      fbb,  //
                      fbb.CreateVector(utl::to_vec(
                          begin(stop_seq), end(stop_seq),
                          [&](trip::stop_identity const& s) {
                            return get_or_create_stop(std::get<0>(s));
                          })),
                      fbb.CreateVector(utl::to_vec(
                          begin(stop_seq), end(stop_seq),
                          [](trip::stop_identity const& s) {
                            return static_cast<uint8_t>(std::get<1>(s) ? 1u
                                                                       : 0u);
                          })),
                      fbb.CreateVector(utl::to_vec(
                          begin(stop_seq), end(stop_seq),
                          [](trip::stop_identity const& s) {
                            return static_cast<uint8_t>(std::get<2>(s) ? 1u
                                                                       : 0u);
                          })));
                }),
            fbb.CreateString(serialize_bitset(*t->service_)),
            fbb.CreateVector(repeat_n(
                CreateSection(
                    fbb, get_or_create_category(t->route_->type_),
                    get_or_create_provider(t->route_->agency_), 0,
                    get_or_create_str(t->route_->short_name_),
                    fbb.CreateVector(std::vector<Offset<Attribute>>()),
                    CreateDirection(fbb, 0, get_or_create_str(t->headsign_))),
                stop_seq.size() - 1)),
            0,
            fbb.CreateVector(std::accumulate(
                begin(t->stop_times_), end(t->stop_times_), std::vector<int>(),
                [](std::vector<int>& times,
                   flat_map<stop_time>::entry_t const& st) {
                  times.emplace_back(st.second.arr_.time_);
                  times.emplace_back(st.second.dep_.time_);
                  return times;
                })));
      }));

  auto const footpaths = fbb.CreateVector(std::accumulate(
      begin(transfers), end(transfers), std::vector<Offset<Footpath>>(),
      [&](std::vector<Offset<Footpath>>& footpaths,
          std::pair<stop_pair, transfer> const& t) {
        if (t.second.type_ == transfer::TIMED_TRANSFER) {
          footpaths.emplace_back(CreateFootpath(
              fbb, get_or_create_stop(t.first.first),
              get_or_create_stop(t.first.second), t.second.minutes_));
        }
        return footpaths;
      }));

  fbb.Finish(CreateSchedule(
      fbb, output_services, fbb.CreateVector(values(fbs_stations)),
      fbb.CreateVector(values(fbs_routes)), &interval, footpaths,
      fbb.CreateVector(std::vector<Offset<RuleService>>())));
}

}  // namespace gtfs
}  // namespace loader
}  // namespace motis
