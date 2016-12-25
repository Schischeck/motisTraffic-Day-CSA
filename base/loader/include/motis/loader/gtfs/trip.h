#pragma once

#include <map>
#include <string>

#include "motis/loader/gtfs/flat_map.h"
#include "motis/loader/gtfs/route.h"
#include "motis/loader/gtfs/services.h"
#include "motis/loader/gtfs/stop.h"
#include "motis/loader/loaded_file.h"

namespace motis {
namespace loader {
namespace gtfs {

struct stop_time {
  stop_time() = default;
  stop_time(stop const* s, std::string headsign, int arr_time, bool out_allowed,
            int dep_time, bool in_allowed)
      : stop_(s),
        headsign_(std::move(headsign)),
        arr_({arr_time, out_allowed}),
        dep_({dep_time, in_allowed}) {}

  struct ev {
    int time_;
    bool in_out_allowed_;
  };

  stop const* stop_;
  std::string headsign_;
  ev arr_, dep_;
};

struct trip {
  using stop_identity = std::tuple<stop const*, bool, bool>;
  using stop_seq = std::vector<stop_identity>;

  trip(route const* route, bitfield const* service, std::string headsign)
      : route_(route), service_(service), headsign_(std::move(headsign)) {}

  stop_seq stops() {
    return utl::to_vec(begin(stop_times_), end(stop_times_),
                       [](flat_map<stop_time>::entry_t const& e) {
                         return std::make_tuple(e.second.stop_,
                                                e.second.arr_.in_out_allowed_,
                                                e.second.dep_.in_out_allowed_);
                       });
  }

  route const* route_;
  bitfield const* service_;
  std::string headsign_;
  flat_map<stop_time> stop_times_;
};

using trip_map = std::map<std::string, std::unique_ptr<trip>>;

trip_map read_trips(loaded_file, route_map const&, services const&);

}  // namespace gtfs
}  // namespace loader
}  // namespace motis
