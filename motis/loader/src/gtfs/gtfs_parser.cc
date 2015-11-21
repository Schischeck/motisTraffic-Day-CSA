#include "motis/loader/gtfs/gtfs_parser.h"

#include "boost/filesystem.hpp"

#include "range/v3/view/transform.hpp"
#include "range/v3/view/remove_if.hpp"
#include "range/v3/view/repeat_n.hpp"
#include "range/v3/view/concat.hpp"
#include "range/v3/view/single.hpp"
#include "range/v3/view/join.hpp"
#include "range/v3/view/map.hpp"

#include "motis/core/common/date_util.h"
#include "motis/loader/gtfs/files.h"
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
#include "motis/schedule-format/Schedule_generated.h"

using namespace ranges;
using namespace flatbuffers;
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

void gtfs_parser::parse(fs::path const& root, FlatBufferBuilder& fbb) {
  auto agencies = read_agencies(loaded_file(root / AGENCY_FILE));
  auto stops = read_stops(loaded_file(root / STOPS_FILE));
  auto routes = read_routes(loaded_file(root / ROUTES_FILE), agencies);
  auto calendar = read_calendar(loaded_file(root / CALENDAR_FILE));
  auto dates = read_calendar_date(loaded_file(root / CALENDAR_DATES_FILE));
  auto services = traffic_days(calendar, dates);
  auto trips = read_trips(loaded_file(root / TRIPS_FILE), routes, services);
  auto transfers = read_transfers(loaded_file(root / TRANSFERS_FILE), stops);
  read_stop_times(loaded_file(root / STOP_TIMES_FILE), trips, stops);

  std::map<int, Offset<Category>> fbs_categories;
  std::map<agency const*, Offset<Provider>> fbs_providers;
  std::map<std::string, Offset<String>> fbs_strings;
  std::map<stop const*, Offset<Station>> fbs_stations;
  std::map<trip::stop_seq, Offset<Route>> fbs_routes;
  std::vector<Offset<Service>> fbs_services;

  auto get_or_create_stop = [&](stop const* s) {
    return get_or_create(fbs_stations, s, [&]() {
      return CreateStation(fbb, fbb.CreateString(s->name_),
                           fbb.CreateString(s->name_), s->lat_, s->lng_, 5,
                           fbb.CreateVector(std::vector<Offset<String>>()));
    });
  };

  auto get_or_create_category = [&](int type) {
    return get_or_create(fbs_categories, type, [&]() {
      return CreateCategory(fbb, fbb.CreateString(route::s_types_.at(type)),
                            CategoryOutputRule_CATEGORY_AND_TRAIN_NUM);
    });
  };

  auto get_or_create_provider = [&](agency const* a) {
    return get_or_create(fbs_providers, a, [&]() {
      return CreateProvider(fbb, fbb.CreateString(a->id_),
                            fbb.CreateString(a->name_), fbb.CreateString(""));
    });
  };

  auto get_or_create_str = [&](std::string const& s) {
    return get_or_create(fbs_strings, s, [&]() { return fbb.CreateString(s); });
  };

  Interval interval(to_unix_time(services.first_day),
                    to_unix_time(services.last_day));
  fbb.Finish(CreateSchedule(
      fbb,
      fbb.CreateVector(
          view::values(trips) |
          view::transform([&](std::unique_ptr<trip> const& t) {
            auto const stop_seq = t->stops();
            return CreateService(
                fbb,
                get_or_create(
                    fbs_routes, stop_seq,
                    [&]() {
                      return CreateRoute(
                          fbb,  //
                          fbb.CreateVector(view::all(stop_seq) |
                                           view::transform(get_n<0>()) |
                                           view::transform([&](stop const* s) {
                                             return get_or_create_stop(s);
                                           }) |
                                           to_vector),
                          fbb.CreateVector(view::all(stop_seq) |
                                           view::transform(get_n<1>()) |
                                           view::transform([&](bool t) {
                                             return (uint8_t)(t ? 1 : 0);
                                           }) |
                                           to_vector),
                          fbb.CreateVector(view::all(stop_seq) |
                                           view::transform(get_n<2>()) |
                                           view::transform([&](bool t) {
                                             return (uint8_t)(t ? 1 : 0);
                                           }) |
                                           to_vector));
                    }),
                fbb.CreateString(serialize_bitset(*t->service_)),
                fbb.CreateVector(
                    view::repeat_n(
                        CreateSection(
                            fbb, get_or_create_category(t->route_->type_),
                            get_or_create_provider(t->route_->agency_), 0,
                            get_or_create_str(t->route_->short_name_),
                            fbb.CreateVector(std::vector<Offset<Attribute>>()),
                            CreateDirection(fbb, 0,
                                            get_or_create_str(t->headsign_))),
                        stop_seq.size() - 1) |
                    to_vector),
                0,
                fbb.CreateVector(
                    view::all(t->stop_times_) |
                    view::transform([](flat_map<stop_time>::entry_t const& st) {
                      return view::concat(view::single(st.second.arr_.time_),
                                          view::single(st.second.dep_.time_));
                    }) |
                    view::join | to_vector));
          }) |
          to_vector),
      fbb.CreateVector(values(fbs_stations)),
      fbb.CreateVector(values(fbs_routes)), &interval,
      fbb.CreateVector(
          view::all(transfers) |
          view::remove_if([](std::pair<stop_pair, transfer> const& t) {
            return t.second.type_ != transfer::TIMED_TRANSFER;
          }) |
          view::transform([&](std::pair<stop_pair, transfer> const& t) {
            return CreateFootpath(fbb, get_or_create_stop(t.first.first),
                                  get_or_create_stop(t.first.second),
                                  t.second.minutes_);
          }) |
          to_vector),
      fbb.CreateVector(std::vector<Offset<RuleService>>())));
}

}  // gtfs
}  // loader
}  // motis
