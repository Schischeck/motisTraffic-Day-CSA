#pragma once

#include "motis/core/common/logging.h"

#include "motis/csa/csa_journey.h"
#include "motis/csa/csa_reconstruction.h"
#include "motis/csa/csa_search_shared.h"
#include "motis/csa/csa_statistics.h"
#include "motis/csa/csa_timetable.h"

#include <algorithm>
#include <array>
#include <iostream>
#include <map>

#include "utl/erase_if.h"

namespace motis::csa::cpu {

template <search_dir Dir>
struct csa_search {
  static constexpr auto INVALID =
      Dir == search_dir::FWD ? time(std::numeric_limits<int16_t>::max(), 1439)
                             : time(std::numeric_limits<int16_t>::min(), 0);

  csa_search(csa_timetable const& tt, time const& start_time,
             csa_statistics& stats)
      : tt_(tt),
        start_time_(start_time),
        arrival_time_(
            tt.stations_.size(),
            array_maker<time, MAX_TRANSFERS + 1>::make_array(INVALID)),
        trip_reachable_(tt.trip_count_,
                        array_maker<con_idx_t, MAX_TRANSFERS + 1>::make_array(
                            std::numeric_limits<con_idx_t>::max())),
        stats_(stats) {}

  void add_start(csa_station const& station, time const& initial_duration) {
    auto const station_arrival = Dir == search_dir::FWD
                                     ? start_time_ + initial_duration
                                     : start_time_ - initial_duration;
    start_times_[station.id_] = station_arrival;
    arrival_time_[station.id_][0] = station_arrival;
    stats_.start_count_++;
    expand_footpaths(station, station_arrival, 0);
  }

  void
  search() {  // TODO / NOTICE: BWD not implemented/adjusted to bitfild version
    if (start_time_ > tt_.last_event_) {
      return;
    }
    auto const& connections =
        Dir == search_dir::FWD ? tt_.fwd_connections_ : tt_.bwd_connections_;

    auto search_day = start_time_.day();

    csa_connection const start_at{start_time_};
    auto const first_connection = std::lower_bound(
        begin(connections), end(connections), start_at,
        [&](csa_connection const& a, csa_connection const& b) {
          return Dir == search_dir::FWD ? a.departure_ < b.departure_
                                        : a.arrival_ > b.arrival_;
        });

    auto const time_limit =
        Dir == search_dir::FWD
            ? std::min(start_time_ + MAX_TRAVEL_TIME, tt_.last_event_)
            : start_time_ - MAX_TRAVEL_TIME;

    for (auto it = first_connection; true; ++it) {
      if (it == end(connections)) {
        it = begin(connections);
        search_day++;
      }
      auto const& con = *it;

      auto const time_limit_reached =
          Dir == search_dir::FWD ? time(search_day, con.departure_) > time_limit
                                 : time(search_day, con.arrival_) < time_limit;
      if (time_limit_reached) {
        break;
      }

      if (!con.traffic_days_->test(search_day)) {
        continue;
      }
      auto const con_departure_time = time(search_day, con.departure_);
      auto const con_arrival_time = time(search_day, con.arrival_);
      auto& trip_reachable = trip_reachable_[con.trip_];
      auto const& from_arrival_time = arrival_time_[con.from_station_];
      auto const& to_arrival_time = arrival_time_[con.to_station_];

      stats_.connections_scanned_++;
      for (auto transfers = 0; transfers < MAX_TRANSFERS; ++transfers) {
        auto const via_trip =
            trip_reachable[transfers] <= con.trip_con_idx_;  // NOLINT
        auto const via_station =
            Dir == search_dir::FWD
                ? (from_arrival_time[transfers] <= con_departure_time  // NOLINT
                   && con.from_in_allowed_)
                : (to_arrival_time[transfers] >= con_arrival_time &&  // NOLINT
                   con.to_out_allowed_);
        if (via_trip || via_station) {
          if (!via_trip) {
            trip_reachable[transfers] = con.trip_con_idx_;  // NOLINT
          }
          auto const update =
              Dir == search_dir::FWD
                  ? con_arrival_time <
                            to_arrival_time[transfers + 1] &&  // NOLINT
                        con.to_out_allowed_
                  : (con_departure_time >=
                     from_arrival_time[transfers + 1]) &&  // NOLINT
                        con.from_in_allowed_;
          if (update) {
            stats_.footpaths_expanded_++;
            if (Dir == search_dir::FWD) {
              expand_footpaths(tt_.stations_[con.to_station_], con_arrival_time,
                               transfers + 1);
            } else {
              expand_footpaths(tt_.stations_[con.from_station_],
                               con_departure_time, transfers + 1);
            }
          }
        }
      }
    }
  }

  void expand_footpaths(csa_station const& station, time const& station_arrival,
                        int transfers) {
    if (Dir == search_dir::FWD) {
      for (auto const& fp : station.footpaths_) {
        auto const fp_arrival = station_arrival + fp.duration_;
        if (arrival_time_[fp.to_station_][transfers] > fp_arrival) {
          arrival_time_[fp.to_station_][transfers] = fp_arrival;
        }
      }
    } else {
      for (auto const& fp : station.incoming_footpaths_) {
        auto const fp_arrival = station_arrival - fp.duration_;
        if (arrival_time_[fp.from_station_][transfers] < fp_arrival) {
          arrival_time_[fp.from_station_][transfers] = fp_arrival;
        }
      }
    }
  }

  std::vector<csa_journey> get_results(csa_station const& station) {
    std::vector<csa_journey> journeys;
    auto const& station_arrival = arrival_time_[station.id_];
    for (auto i = 0; i <= MAX_TRANSFERS; ++i) {
      auto const arrival_time = station_arrival[i];  // NOLINT
      if (arrival_time != INVALID) {
        csa_reconstruction<Dir, decltype(arrival_time_),
                           decltype(trip_reachable_)>{
            tt_, start_times_, arrival_time_, trip_reachable_}
            .extract_journey(journeys.emplace_back(Dir, start_time_,
                                                   arrival_time, i, &station));
      }
    }
    return journeys;
  }

  csa_timetable const& tt_;
  time start_time_;
  std::map<station_id, time> start_times_;
  std::vector<std::array<time, MAX_TRANSFERS + 1>> arrival_time_;  // S
  std::vector<std::array<con_idx_t, MAX_TRANSFERS + 1>>
      trip_reachable_;  // T  connection index from which the trip can be used
  csa_statistics& stats_;
};

}  // namespace motis::csa::cpu
