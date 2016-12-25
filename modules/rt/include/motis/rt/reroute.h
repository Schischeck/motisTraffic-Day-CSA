#pragma once

#include "utl/to_vec.h"

#include "motis/core/schedule/graph_build_utils.h"
#include "motis/core/schedule/schedule.h"
#include "motis/core/conv/event_type_conv.h"

#include "motis/rt/connection_builder.h"
#include "motis/rt/event_resolver.h"
#include "motis/rt/find_trip_fuzzy.h"
#include "motis/rt/in_out_allowed.h"
#include "motis/rt/incoming_edges.h"

namespace motis {
namespace rt {

enum class status {
  OK,
  TRIP_NOT_FOUND,
  EVENT_COUNT_MISMATCH,
  STATION_MISMATCH,
  EVENT_ORDER_MISMATCH,
  RULE_SERVICE_REROUTE_NOT_SUPPORTED
};

struct schedule_event {
  schedule_event(primary_trip_id trp_id, uint32_t station_idx,
                 motis::time schedule_time, event_type ev_type)
      : trp_id_(std::move(trp_id)),
        station_idx_(station_idx),
        schedule_time_(schedule_time),
        ev_type_(ev_type) {}

  friend bool operator<(schedule_event const& a, schedule_event const& b) {
    return std::tie(a.trp_id_, a.station_idx_, a.schedule_time_, a.ev_type_) <
           std::tie(b.trp_id_, b.station_idx_, b.schedule_time_, b.ev_type_);
  }

  friend bool operator==(schedule_event const& a, schedule_event const& b) {
    return std::tie(a.trp_id_, a.station_idx_, a.schedule_time_, a.ev_type_) ==
           std::tie(b.trp_id_, b.station_idx_, b.schedule_time_, b.ev_type_);
  }

  primary_trip_id trp_id_;
  uint32_t station_idx_;
  motis::time schedule_time_;
  event_type ev_type_;
};

struct reroute_event : public event_info {
  enum class type { ORIGINAL_EVENT, ADDITIONAL };

  // ORIGINAL_EVENT
  reroute_event(ev_key k, motis::time schedtime, delay_info* di)
      : event_info(k.get_station_idx(), schedtime, k.ev_type_),
        type_(type::ORIGINAL_EVENT),
        k_(k),
        in_out_(get_in_out_allowed(k.get_node())),
        di_(di),
        additional_event_(nullptr) {}

  // ADDITIONAL
  reroute_event(event_info ev_info, ris::ReroutedEvent const* additional_event,
                delay_info* di)
      : event_info(std::move(ev_info)),
        type_(type::ADDITIONAL),
        in_out_(true, true),
        di_(di),
        additional_event_(additional_event) {}

  type type_;

  // ORIGINAL_EVENT: original event
  // ADDITIONAL: invalid event
  ev_key k_;

  // ORIGINAL_EVENT: original in allowed / out allowed setting
  // ADDITIONAL: (true, true)
  in_out_allowed in_out_;

  // ORIGINAL_EVENT: nullptr | delay info, if available
  // ADDITIONAL: nullptr | delay info, if stored
  delay_info* di_;

  // ORIGINAL_EVENT: nullptr
  // ADDITIONAL: points to fbs message
  ris::ReroutedEvent const* additional_event_;
};

struct section {
  section(light_connection lcon, station_node* from, station_node* to,
          reroute_event dep, reroute_event arr)
      : lcon_(std::move(lcon)),
        from_(from),
        to_(to),
        dep_(std::move(dep)),
        arr_(std::move(arr)) {}

  light_connection lcon_;
  station_node *from_, *to_;
  reroute_event dep_, arr_;
};

inline delay_info* get_stored_delay_info(
    schedule const& sched,
    std::map<schedule_event, delay_info*> const& cancelled_delays,
    trip const* trp, ris::ReroutedEvent const* ev) {
  auto it = cancelled_delays.find(schedule_event{
      trp->id_.primary_,
      get_station(sched, ev->base()->base()->station_id()->str())->index_,
      unix_to_motistime(sched, ev->base()->base()->schedule_time()),
      from_fbs(ev->base()->base()->type())});
  return (it == end(cancelled_delays)) ? nullptr : it->second;
}

inline void store_cancelled_delays(
    schedule const& sched, trip const* trp,
    std::vector<boost::optional<ev_key>> const& del_evs,
    std::map<schedule_event, delay_info*>& cancelled_delays) {
  for (auto const& ev : del_evs) {
    if (!ev) {
      continue;
    }

    auto it = sched.graph_to_delay_info_.find(*ev);
    if (it != end(sched.graph_to_delay_info_)) {
      cancelled_delays.emplace(
          schedule_event{trp->id_.primary_, ev->get_station_idx(),
                         it->second->get_schedule_time(), ev->ev_type_},
          it->second);
    }
  }
}

inline void add_additional_events(
    schedule& sched,
    std::map<schedule_event, delay_info*> const& cancelled_delays,
    trip const* trp,
    flatbuffers::Vector<flatbuffers::Offset<ris::ReroutedEvent>> const*
        new_events,
    std::vector<reroute_event>& events) {
  auto const new_opt_evs = resolve_event_info(
      sched, utl::to_vec(*new_events, [](ris::ReroutedEvent const* ev) {
        return ev->base()->base();
      }));

  for (unsigned i = 0; i < new_opt_evs.size(); ++i) {
    auto const& ev = new_opt_evs[i];
    if (!ev) {
      continue;
    }

    auto const new_ev = new_events->Get(i);
    events.emplace_back(*ev, new_ev, get_stored_delay_info(
                                         sched, cancelled_delays, trp, new_ev));
  }
}

inline void add_if_not_deleted(
    schedule const& sched, std::vector<boost::optional<ev_key>> const& deleted,
    ev_key const& k, std::vector<reroute_event>& events) {
  if (std::find_if(begin(deleted), end(deleted),
                   [&k](boost::optional<ev_key> const& del_ev) {
                     return del_ev && *del_ev == k;
                   }) != end(deleted)) {
    return;
  }

  auto const di_it = sched.graph_to_delay_info_.find(k);
  auto const di =
      di_it == end(sched.graph_to_delay_info_) ? nullptr : di_it->second;
  auto const schedtime = di ? di->get_schedule_time() : k.get_time();
  events.emplace_back(k, schedtime, di);
}

inline void add_not_deleted_trip_events(
    schedule& sched, std::vector<boost::optional<ev_key>> const& deleted,
    trip const* trp, std::vector<reroute_event>& events) {
  for (auto const& trp_e : *trp->edges_) {
    auto const e = trp_e.get_edge();
    add_if_not_deleted(sched, deleted,
                       ev_key{e, trp->lcon_idx_, event_type::DEP}, events);
    add_if_not_deleted(sched, deleted,
                       ev_key{e, trp->lcon_idx_, event_type::ARR}, events);
  }
}

inline status check_events(std::vector<reroute_event> const& events) {
  if (events.empty() || events.size() % 2 != 0) {
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

inline uint16_t get_track(schedule& sched, reroute_event const& ev) {
  switch (ev.type_) {
    case reroute_event::type::ADDITIONAL:
      return get_track(sched, ev.additional_event_->base()->track()->str());
    case reroute_event::type::ORIGINAL_EVENT: {
      auto const& fcon = ev.k_.lcon()->full_con_;
      return ev.ev_type_ == event_type::DEP ? fcon->d_track_ : fcon->a_track_;
    }
    default: return 0;
  }
}

inline connection_info const* get_con_info(
    schedule& sched,
    std::map<connection_info, connection_info const*> con_infos,
    reroute_event const& ev) {
  switch (ev.type_) {
    case reroute_event::type::ADDITIONAL: {
      auto const base = ev.additional_event_->base();
      return get_con_info(sched, con_infos, base->category()->str(),
                          base->base()->line_id()->str(),
                          base->base()->service_num());
    }
    case reroute_event::type::ORIGINAL_EVENT:
      return ev.k_.lcon()->full_con_->con_info_;
    default: return nullptr;
  }
}

inline std::vector<section> build_trip_from_events(
    schedule& sched, std::vector<reroute_event> const& events) {
  auto const get_time = [](reroute_event const& ev) {
    return ev.di_ ? ev.di_->get_current_time() : ev.sched_time_;
  };

  std::vector<section> sections;
  std::map<connection_info, connection_info const*> con_infos;
  for (auto it = std::begin(events); it != std::end(events); ++it, ++it) {
    auto const& dep = *it;
    auto const& arr = *std::next(it);

    auto const dep_station = sched.station_nodes_.at(dep.station_idx_).get();
    auto const arr_station = sched.station_nodes_.at(arr.station_idx_).get();

    if (dep.type_ == reroute_event::type::ORIGINAL_EVENT &&
        arr.type_ == reroute_event::type::ORIGINAL_EVENT &&
        dep.k_.get_opposite() == arr.k_) {
      sections.emplace_back(*dep.k_.lcon(), dep_station, arr_station, dep, arr);
    } else {
      sections.emplace_back(
          light_connection{
              get_time(dep), get_time(arr),
              get_full_con(sched, get_con_info(sched, con_infos, dep),
                           get_track(sched, dep), get_track(sched, arr))},
          dep_station, arr_station, dep, arr);
    }
  }
  return sections;
}

inline std::vector<trip::route_edge> build_route(
    schedule& sched, std::vector<section> const& sections,
    std::map<node const*, std::vector<edge*>>& incoming) {
  auto const route_id = sched.route_count_++;

  std::vector<trip::route_edge> trip_edges;
  node* prev_route_node = nullptr;
  for (auto& s : sections) {
    auto const from_station_transfer_time =
        sched.stations_.at(s.from_->id_)->transfer_time_;
    auto const to_station_transfer_time =
        sched.stations_.at(s.to_->id_)->transfer_time_;

    auto const from_route_node =
        prev_route_node ? prev_route_node
                        : build_route_node(route_id, sched.node_count_++,
                                           s.from_, from_station_transfer_time,
                                           s.dep_.in_out_.in_allowed_,
                                           s.dep_.in_out_.out_allowed_);
    auto const to_route_node = build_route_node(
        route_id, sched.node_count_++, s.to_, to_station_transfer_time,
        s.arr_.in_out_.in_allowed_, s.arr_.in_out_.out_allowed_);

    from_route_node->edges_.push_back(
        make_route_edge(from_route_node, to_route_node, {s.lcon_}));

    auto const route_edge = &from_route_node->edges_.back();
    incoming[to_route_node].push_back(route_edge);
    trip_edges.emplace_back(route_edge);

    prev_route_node = to_route_node;
  }

  return trip_edges;
}

inline std::set<station_node*> get_station_nodes(
    std::vector<section> const& sections) {
  std::set<station_node*> station_nodes;
  for (auto& c : sections) {
    station_nodes.insert(c.from_);
    station_nodes.insert(c.to_);
  }
  return station_nodes;
}

inline void update_delay_infos(
    std::vector<reroute_event>& events,
    std::vector<trip::route_edge> const& trip_edges) {
  auto const update_di = [](reroute_event& ev, ev_key const& new_ev) {
    if (ev.di_ != nullptr) {
      ev.di_->set_ev_key(new_ev);
    }
  };

  for (auto i = 0u; i < events.size(); i += 2) {
    auto const e = trip_edges[i / 2].get_edge();
    update_di(events[i], ev_key{e, 0, event_type::DEP});
    update_di(events[i + 1], ev_key{e, 0, event_type::ARR});
  }
}

inline void update_trip(schedule& sched, trip* trp,
                        std::vector<trip::route_edge> const& trip_edges) {
  for (auto const& trp_e : *trp->edges_) {
    auto const e = trp_e.get_edge();
    e->m_.route_edge_.conns_[trp->lcon_idx_].valid_ = false;
  }

  std::vector<trip*> trps = {trp};
  auto merged_trps_idx = sched.merged_trips_.size();
  sched.merged_trips_.emplace_back(std::make_unique<std::vector<trip*>>(trps));
  for (auto const& e : trip_edges) {
    e->m_.route_edge_.conns_[0].trips_ = merged_trps_idx;
  }

  sched.trip_edges_.emplace_back(
      std::make_unique<std::vector<trip::route_edge>>(trip_edges));
  trp->edges_ = sched.trip_edges_.back().get();
  trp->lcon_idx_ = 0;
}

inline std::pair<status, trip const*> reroute(
    schedule& sched, std::map<schedule_event, delay_info*>& cancelled_delays,
    ris::RerouteMessage const* msg) {
  auto const trp =
      const_cast<trip*>(find_trip_fuzzy(sched, msg->trip_id()));  // NOLINT
  if (trp == nullptr) {
    return {status::TRIP_NOT_FOUND, nullptr};
  }

  for (auto const& e : *trp->edges_) {
    if (get_lcon(e, trp->lcon_idx_).full_con_->con_info_->merged_with_ ||
        std::any_of(
            begin(e->from_->incoming_edges_), end(e->from_->incoming_edges_),
            [](edge const* e) { return e->type() == edge::THROUGH_EDGE; }) ||
        std::any_of(
            begin(e->to_->edges_), end(e->to_->edges_),
            [](edge const& e) { return e.type() == edge::THROUGH_EDGE; })) {
      return {status::RULE_SERVICE_REROUTE_NOT_SUPPORTED, nullptr};
    }
  }

  auto evs = std::vector<reroute_event>{};
  auto const del_evs = resolve_events(
      sched, trp, utl::to_vec(*msg->cancelled_events(),
                              [](ris::Event const* ev) { return ev; }));
  add_additional_events(sched, cancelled_delays, trp, msg->new_events(), evs);
  add_not_deleted_trip_events(sched, del_evs, trp, evs);
  std::sort(begin(evs), end(evs));
  auto check_result = check_events(evs);
  if (check_result != status::OK) {
    return {check_result, trp};
  }

  auto const sections = build_trip_from_events(sched, evs);
  auto const station_nodes = get_station_nodes(sections);
  auto incoming = incoming_non_station_edges(station_nodes);
  auto const trip_edges = build_route(sched, sections, incoming);
  add_incoming_station_edges(station_nodes, incoming);
  rebuild_incoming_edges(station_nodes, incoming);
  update_delay_infos(evs, trip_edges);
  update_trip(sched, trp, trip_edges);
  store_cancelled_delays(sched, trp, del_evs, cancelled_delays);

  return {status::OK, trp};
}

}  // namespace rt
}  // namespace motis
