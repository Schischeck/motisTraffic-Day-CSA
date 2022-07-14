#pragma once

#include "boost/filesystem.hpp"

#include "motis/core/common/logging.h"

#include "motis/core/access/service_access.h"

#include "motis/csa/csa_search_shared.h"
#include "motis/csa/csa_timetable.h"

#include "utl/pipes.h"
#include "utl/verify.h"

#define TFMT "%02d:%02d.%d"
#define FMT_TIME(t)                                             \
  ((t) >= INVALID_TIME ? 0 : (((t) % MINUTES_A_DAY) / 60)),     \
      ((t) >= INVALID_TIME ? 0 : (((t) % MINUTES_A_DAY) % 60)), \
      ((t) >= INVALID_TIME ? 0 : ((t) / MINUTES_A_DAY))

namespace motis::csa {

struct journey_pointer {
  journey_pointer() = default;
  journey_pointer(csa_connection const* enter_con,
                  csa_connection const* exit_con, footpath const* footpath,
                  time const& enter_con_dep_time, time const& exit_con_arr_time)
      : enter_con_(enter_con),
        exit_con_(exit_con),
        footpath_(footpath),
        enter_con_dep_time_(enter_con_dep_time),
        exit_con_arr_time_(exit_con_arr_time) {}

  bool valid() const {
    return enter_con_ != nullptr && exit_con_ != nullptr &&
           footpath_ != nullptr;
  }

  csa_connection const* enter_con_{nullptr};
  csa_connection const* exit_con_{nullptr};
  footpath const* footpath_{nullptr};
  time enter_con_dep_time_;
  time exit_con_arr_time_;
};

template <search_dir Dir, typename ArrivalTimes, typename TripReachable>
struct csa_reconstruction {

  static constexpr auto INVALID =
      Dir == search_dir::FWD ? time(std::numeric_limits<int16_t>::max(), 1439)
                             : time(std::numeric_limits<int16_t>::min(), 0);

  csa_reconstruction(csa_timetable const& tt,
                     std::map<station_id, time> const& start_times,
                     ArrivalTimes const& arrival_time,
                     TripReachable const& trip_reachable)
      : tt_(tt),
        start_times_(start_times),
        arrival_time_(arrival_time),
        trip_reachable_(trip_reachable) {}

  inline bool is_start(station_id station) const {
    return start_times_.find(station) != end(start_times_);
  }

  void extract_journey(csa_journey& j) {
    if (j.is_reconstructed()) {
      return;
    }

    auto stop = j.destination_station_;
    auto transfers = j.transfers_;
    for (; transfers > 0; --transfers) {
      auto jp = get_journey_pointer(*stop, transfers);
      if (jp.valid()) {
        if (transfers == j.transfers_ &&
            jp.footpath_->from_station_ != jp.footpath_->to_station_) {
          if (auto const con_arr_jp =
                  look_for_conn_arrival_within_transfer_time(stop, transfers);
              con_arr_jp.valid()) {
            jp = con_arr_jp;
          }
        }

        if (jp.footpath_->from_station_ != jp.footpath_->to_station_) {
          if (Dir == search_dir::FWD) {
            j.edges_.emplace_back(
                &tt_.stations_[jp.footpath_->from_station_],
                &tt_.stations_[jp.footpath_->to_station_],
                jp.exit_con_arr_time_,
                jp.exit_con_arr_time_ + jp.footpath_->duration_, -1);
          } else {
            j.edges_.emplace_back(
                &tt_.stations_[jp.footpath_->from_station_],
                &tt_.stations_[jp.footpath_->to_station_],
                jp.enter_con_dep_time_ - jp.footpath_->duration_,
                jp.enter_con_dep_time_, -1);
          }
        }
        assert(jp.enter_con_->trip_ == jp.exit_con_->trip_);
        auto const& trip_cons = tt_.trip_to_connections_[jp.exit_con_->trip_];
        auto const add_trip_edge = [&](csa_connection const* con) {
          auto const enter = con == jp.enter_con_;
          auto const exit = con == jp.exit_con_;
          utl::new_verify(con->light_con_ != nullptr,
                          "invalid light connection");
          auto const con_dep_day = jp.enter_con_dep_time_.day() +
                                   con->day_offset_ -
                                   jp.enter_con_->day_offset_;
          auto const arr_mam_time = time(con->arrival_);
          j.edges_.emplace_back(
              con->light_con_, &tt_.stations_[con->from_station_],
              &tt_.stations_[con->to_station_], enter, exit,
              time(con_dep_day, con->departure_),
              time(con_dep_day + arr_mam_time.day(), arr_mam_time.mam()));
        };
        if (Dir == search_dir::FWD) {
          auto in_trip = false;
          for (int i = static_cast<int>(trip_cons.size()) - 1; i >= 0; --i) {
            auto const con = trip_cons[i];
            if (con == jp.exit_con_) {
              in_trip = true;
            }
            if (in_trip) {
              add_trip_edge(con);
            }
            if (con == jp.enter_con_) {
              break;
            }
          }
          stop = &tt_.stations_[jp.enter_con_->from_station_];
        } else {
          auto in_trip = false;
          for (auto const& con : trip_cons) {
            if (con == jp.enter_con_) {
              in_trip = true;
            }
            if (in_trip) {
              add_trip_edge(con);
            }
            if (con == jp.exit_con_) {
              break;
            }
          }
          stop = &tt_.stations_[jp.exit_con_->to_station_];
        }
        j.start_station_ = stop;
      } else {
        if (!is_start(stop->id_)) {
          if (transfers != 0) {
            LOG(motis::logging::warn)
                << "csa extract journey: adding final footpath "
                   "with transfers="
                << transfers;
          }
          add_final_footpath(j, stop, transfers);
        }
        break;
      }
    }
    if (transfers == 0 && !is_start(stop->id_)) {
      add_final_footpath(j, stop, transfers);
    }
    if (Dir == search_dir::FWD) {
      std::reverse(begin(j.edges_), end(j.edges_));
    }
  }

  journey_pointer look_for_conn_arrival_within_transfer_time(
      csa_station const* stop, int transfers) {
    for (auto t = 0; t <= tt_.stations_[stop->id_].transfer_time_.ts(); ++t) {
      auto const offset = Dir == search_dir::FWD ? t : -t;
      auto const jp = get_journey_pointer(
          *stop, transfers, arrival_time_[stop->id_][transfers] + offset);
      if (jp.valid()) {
        return jp;
      }
    }
    return {};
  }

  void add_final_footpath(csa_journey& j, csa_station const* stop,
                          int transfers) {
    assert(transfers == 0);
    auto const fp_arrival = arrival_time_[stop->id_][transfers];
    if (Dir == search_dir::FWD) {
      for (auto const& fp : stop->incoming_footpaths_) {
        if (fp.from_station_ == fp.to_station_) {
          continue;
        }
        auto const fp_departure = fp_arrival - fp.duration_;
        auto const valid_station = is_start(fp.from_station_);
        if (valid_station &&
            fp_departure >= start_times_.at(fp.from_station_)) {
          j.edges_.emplace_back(&tt_.stations_[fp.from_station_],
                                &tt_.stations_[fp.to_station_], fp_departure,
                                fp_arrival, -1);
          j.start_station_ = &tt_.stations_[fp.to_station_];
          break;
        }
      }
    } else {
      for (auto const& fp : stop->footpaths_) {
        if (fp.from_station_ == fp.to_station_) {
          continue;
        }
        auto const fp_departure = fp_arrival + fp.duration_;
        auto const valid_station = is_start(fp.to_station_);
        if (valid_station && fp_departure <= start_times_.at(fp.to_station_)) {
          j.edges_.emplace_back(&tt_.stations_[fp.from_station_],
                                &tt_.stations_[fp.to_station_], fp_arrival,
                                fp_departure, -1);
          j.start_station_ = &tt_.stations_[fp.from_station_];
          break;
        }
      }
    }
  }

  journey_pointer get_journey_pointer(
      csa_station const& station, int transfers,
      time const& station_arrival_override = INVALID) {
    auto const is_override_active = station_arrival_override != INVALID;
    auto const& station_arrival = is_override_active
                                      ? station_arrival_override
                                      : arrival_time_[station.id_][transfers];
    if (Dir == search_dir::FWD) {
      for (auto const& fp : station.incoming_footpaths_) {
        if (is_override_active && fp.from_station_ != fp.to_station_) {
          continue;  // Override => we are looking for connection arrivals.
        }

        auto const& fp_dep_stop = tt_.stations_[fp.from_station_];
        auto const fp_dep_time = station_arrival - fp.duration_;
        for (auto const& exit_con :
             get_exit_candidates(fp_dep_stop, fp_dep_time, transfers)) {

          assert(exit_con->traffic_days_->test(fp_dep_time.day() -
                                               time(exit_con->arrival_).day()));
          assert(fp_dep_time.mam() == time(exit_con->arrival_).mam());

          auto const exit_con_dep_day =
              (fp_dep_time.day() - time(exit_con->arrival_).day());
          for (auto const& enter_con :
               tt_.trip_to_connections_[exit_con->trip_]) {

            auto const enter_con_dep =
                time(exit_con_dep_day -
                         (exit_con->day_offset_ - enter_con->day_offset_),
                     enter_con->departure_);
            if (arrival_time_[enter_con->from_station_][transfers - 1] <=
                    enter_con_dep &&
                enter_con->from_in_allowed_) {
              return {enter_con, exit_con, &fp, enter_con_dep, fp_dep_time};
            }
            if (enter_con == exit_con) {
              break;
            }
          }
        }
      }
    } else {
      for (auto const& fp : station.footpaths_) {
        auto const& fp_arr_stop = tt_.stations_[fp.to_station_];
        auto const fp_arr_time = station_arrival + fp.duration_;
        for (auto const& enter_con :
             get_enter_candidates(fp_arr_stop, fp_arr_time, transfers)) {
          auto const& trip_cons = tt_.trip_to_connections_[enter_con->trip_];
          for (auto i = static_cast<int>(trip_cons.size()) - 1; i >= 0; --i) {
            auto const& exit_con = trip_cons[i];
            auto const exit_arrival =
                arrival_time_[exit_con->to_station_][transfers - 1];
            auto const exit_con_arr_mam_t = time(exit_con->arrival_);
            auto const exit_con_arr_day =
                (fp_arr_time).day() +
                (exit_con->day_offset_ - enter_con->day_offset_) +
                exit_con_arr_mam_t.day();
            auto const exit_con_arr_time =
                time(exit_con_arr_day, exit_con_arr_mam_t.mam());
            if (exit_arrival != INVALID && exit_arrival >= exit_con_arr_time &&
                exit_con->to_out_allowed_) {
              return {enter_con, exit_con, &fp, fp_arr_time, exit_con_arr_time};
            }
            if (exit_con == enter_con) {
              break;
            }
          }
        }
      }
    }
    return {};
  }

  bool not_same_arrival(csa_connection const& con,
                        time const& arrival_time) const {
    auto const con_arr_t = time(con.arrival_);
    return (con_arr_t.mam() != arrival_time.mam()) ||
           (!con.traffic_days_->test(arrival_time.day() - con_arr_t.day()));
  }

  auto get_exit_candidates(csa_station const& arrival_station,
                           time const& arrival_time, int transfers) const {
    return utl::all(arrival_station.incoming_connections_)  //
           | utl::remove_if(
                 [this, arrival_time, transfers](csa_connection const* con) {
                   auto const con_arr_t = time(con->arrival_);
                   auto const con_dep_d = arrival_time.day() - con_arr_t.day();
                   return (con_arr_t.mam() != arrival_time.mam()) ||
                          !con->traffic_days_->test(con_dep_d) ||
                          (trip_reachable_[con->trip_][transfers - 1] >
                           con->trip_con_idx_) ||
                          !con->to_out_allowed_;
                 })  //
           | utl::iterable();
  }

  auto get_enter_candidates(csa_station const& departure_station,
                            time const& departure_time, int transfers) const {
    return utl::all(departure_station.outgoing_connections_)  //
           | utl::remove_if(
                 [this, departure_time, transfers](csa_connection const* con) {
                   return !con->traffic_days_->test(departure_time.day()) ||
                          con->departure_ != departure_time.mam() ||
                          trip_reachable_[con->trip_][transfers - 1] >
                              departure_time.day() ||
                          !con->from_in_allowed_;
                 })  //
           | utl::iterable();
  }

  csa_timetable const& tt_;
  std::map<station_id, time> const& start_times_;
  ArrivalTimes const& arrival_time_;  // S
  TripReachable const&
      trip_reachable_;  // T  connection index from which the trip can be used
};

}  // namespace motis::csa
