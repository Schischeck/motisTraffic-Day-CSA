#pragma once

#include "motis/core/schedule/schedule.h"

#include "motis/rt/event_resolver.h"
#include "motis/rt/find_trip_fuzzy.h"

namespace motis {
namespace rt {

enum class status {
  OK,
  TRIP_NOT_FOUND,
  EVENT_COUNT_MISMATCH,
  STATION_MISMATCH,
  EVENT_ORDER_MISMATCH
};

struct event : public event_info {
  enum class type { TRIP_EVENT, ADDITIONAL };

  // TRIP_EVENT
  event(ev_key k, motis::time sched_time, delay_info* di)
      : event_info(k.get_station_idx(), sched_time, k.ev_type_),
        type_(type::TRIP_EVENT),
        k_(k),
        di_(di),
        additional_event_(nullptr) {}

  // ADDITIONAL
  event(event_info ev_info, ris::ReroutedEvent const* additional_event,
        delay_info* di)
      : event_info(std::move(ev_info)),
        type_(type::ADDITIONAL),
        di_(di),
        additional_event_(additional_event) {}

  type type_;

  // TRIP_EVENT: original full connection
  // ADDITIONAL: nullptr
  ev_key k_;

  // TRIP_EVENT: nullptr | delay info, if available
  // ADDITIONAL: nullptr | delay info, if stored
  delay_info* di_;

  // TRIP_EVENT: nullptr
  // ADDITIONAL: points to fbs message
  ris::ReroutedEvent const* additional_event_;
};

inline delay_info* get_stored_delay_info(schedule& /* s */,
                                         trip const* /* trp */,
                                         ris::ReroutedEvent const* /* ev */) {
  // TODO(felix) implemennt
  return nullptr;
}

inline void add_additional_events(
    schedule& sched, trip const* trp,
    flatbuffers::Vector<flatbuffers::Offset<ris::ReroutedEvent>> const*
        new_events,
    std::vector<event>& events) {
  auto const new_opt_evs = resolve_event_info(
      sched, transform_to_vec(*new_events, [](ris::ReroutedEvent const* ev) {
        return ev->base()->base();
      }));

  for (unsigned i = 0; i < new_opt_evs.size(); ++i) {
    auto const& ev = new_opt_evs[i];
    if (!ev) {
      continue;
    }

    auto const new_ev = new_events->Get(i);
    events.emplace_back(*ev, new_ev, get_stored_delay_info(sched, trp, new_ev));
  }
}

inline void add_if_not_deleted(
    schedule const& sched, std::vector<boost::optional<ev_key>> const& deleted,
    ev_key const& k, std::vector<event>& events) {
  if (std::find_if(begin(deleted), end(deleted),
                   [&k](boost::optional<ev_key> const& del_ev) {
                     return del_ev && *del_ev == k;
                   }) != end(deleted)) {
    return;
  }

  auto const di_it = sched.graph_to_delay_info_.find(k);
  auto const di =
      di_it == end(sched.graph_to_delay_info_) ? nullptr : di_it->second;
  auto const sched_time = di ? di->get_schedule_time() : k.get_time();
  events.emplace_back(k, sched_time, di);
}

inline void add_not_deleted_trip_events(
    schedule& sched, flatbuffers::Vector<flatbuffers::Offset<ris::Event>> const*
                         cancelled_events,
    trip const* trp, std::vector<event>& events) {
  auto const del_evs = resolve_events(
      sched, trp, transform_to_vec(*cancelled_events,
                                   [](ris::Event const* ev) { return ev; }));

  for (auto const& trp_e : *trp->edges_) {
    auto const e = trp_e.get_edge();
    add_if_not_deleted(sched, del_evs,
                       ev_key{e, trp->lcon_idx_, event_type::DEP}, events);
    add_if_not_deleted(sched, del_evs,
                       ev_key{e, trp->lcon_idx_, event_type::ARR}, events);
  }
}

inline status check_events(std::vector<event> const& events) {
  if (events.size() == 0 || events.size() % 2 != 0) {
    return status::EVENT_COUNT_MISMATCH;
  }

  event_type next = event_type::DEP;
  uint32_t arr_station_idx = 0;
  for (auto const& ev : events) {
    if (ev.ev_type_ != next) {
      return status::EVENT_ORDER_MISMATCH;
    }

    if (ev.ev_type_ == event_type::DEP && arr_station_idx != 0 &&
        ev.station_idx_ != arr_station_idx) {
      return status::STATION_MISMATCH;
    }

    next = (next == event_type::DEP) ? event_type::ARR : event_type::DEP;
    arr_station_idx = ev.station_idx_;
  }

  return status::OK;
}

inline uint16_t get_track(schedule& sched, event const& ev) {
  switch (ev.type_) {
    case event::type::ADDITIONAL:
      return get_track(sched, ev.additional_event_->base()->track()->str());
    case event::type::TRIP_EVENT: return ev.k_.lcon()->full_con_->d_track_;
    default: return 0;
  }
}

inline connection_info const* get_con_info(
    schedule& sched,
    std::map<connection_info, connection_info const*> con_infos,
    event const& ev) {
  switch (ev.type_) {
    case event::type::ADDITIONAL: {
      auto const base = ev.additional_event_->base();
      return get_con_info(sched, con_infos, base->category()->str(),
                          base->base()->line_id()->str(),
                          base->base()->service_num());
    }
    case event::type::TRIP_EVENT: return ev.k_.lcon()->full_con_->con_info_;
    default: return nullptr;
  }
}

inline motis::time get_time(event const& ev) {
  switch (ev.type_) {
    case event::type::ADDITIONAL: return ev.sched_time_;
    case event::type::TRIP_EVENT:
      return ev.di_ ? ev.di_->get_current_time() : ev.sched_time_;
    default: return INVALID_TIME;
  }
}

using section = std::tuple<light_connection, station_node*, station_node*>;
inline std::vector<section> build_trip_from_events(
    schedule& sched, std::vector<event> const& events) {
  std::vector<section> sections;
  std::map<connection_info, connection_info const*> con_infos;
  for (auto it = std::begin(events); it != std::end(events); ++it, ++it) {
    auto const& dep = *it;
    auto const& arr = *std::next(it);

    auto const dep_station = sched.station_nodes_.at(dep.station_idx_).get();
    auto const arr_station = sched.station_nodes_.at(arr.station_idx_).get();

    if (dep.type_ == event::type::TRIP_EVENT &&
        arr.type_ == event::type::TRIP_EVENT &&
        dep.k_.get_opposite() == arr.k_) {
      sections.emplace_back(*dep.k_.lcon(), dep_station, arr_station);
    } else {
      sections.emplace_back(
          light_connection{
              get_time(dep), get_time(arr),
              get_full_con(sched, get_con_info(sched, con_infos, dep),
                           get_track(sched, dep), get_track(sched, arr))},
          dep_station, arr_station);
    }
  }
  return sections;
}

inline status reroute(schedule& s, ris::RerouteMessage const* msg,
                      hash_map<ev_key, ev_key>& /* moved_events */) {
  auto const trp = find_trip_fuzzy(s, msg->trip_id());
  if (trp == nullptr) {
    return status::TRIP_NOT_FOUND;
  }

  auto events = std::vector<event>{};
  add_additional_events(s, trp, msg->new_events(), events);
  add_not_deleted_trip_events(s, msg->cancelled_events(), trp, events);
  std::sort(begin(events), end(events));

  auto check_result = check_events(events);
  if (check_result == status::OK) {
    build_trip_from_events(s, events);
  }

  return check_result;
}

}  // namespace rt
}  // namespace motis
