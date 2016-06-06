#include "motis/rt/rt.h"

#include <queue>

#include "boost/program_options.hpp"

#include "motis/core/access/realtime_access.h"
#include "motis/core/access/station_access.h"
#include "motis/core/access/time_access.h"
#include "motis/core/access/trip_access.h"
#include "motis/module/context/get_schedule.h"
#include "motis/module/context/motis_publish.h"
#include "motis/loader/util.h"
#include "motis/rt/delay_propagator.h"
#include "motis/rt/shifted_nodes_msg_builder.h"

namespace po = boost::program_options;
using namespace flatbuffers;
using namespace motis::module;
using motis::ris::RISBatch;

namespace motis {
namespace rt {

struct update {
  update() = default;
  update(uint32_t station_id, delay_info::reason reason, time schedule_time,
         time new_time)
      : station_id_(station_id),
        reason_(reason),
        schedule_time_(schedule_time),
        updated_time_(new_time) {}

  schedule_event sched_ev(primary_trip_id const& id, event_type type) const {
    return {id, station_id_, type, schedule_time_};
  }

  uint32_t station_id_;
  delay_info::reason reason_;
  time schedule_time_, updated_time_;
};

std::vector<update> get_updates(
    schedule const& sched, delay_info::reason const reason,
    Vector<Offset<ris::UpdatedEvent>> const* events) {
  std::vector<update> updates;

  for (auto const& ev : *events) {
    try {
      auto const station_id = ev->base()->station_id()->str();
      auto const station_node = get_station_node(sched, station_id)->id_;
      auto const time = unix_to_motistime(sched, ev->base()->schedule_time());
      auto const upd_time = unix_to_motistime(sched, ev->updated_time());
      if (time != INVALID_TIME && upd_time != INVALID_TIME) {
        updates.emplace_back(station_node, reason, time, upd_time);
      }
    } catch (...) {
      continue;
    }
  }

  return updates;
}

time get_event_time(light_connection const& lcon, event_type const ev_type) {
  return ev_type == event_type::DEP ? lcon.d_time_ : lcon.a_time_;
}

time& get_event_time(light_connection& lcon, event_type const ev_type) {
  return ev_type == event_type::DEP ? lcon.d_time_ : lcon.a_time_;
}

node* get_route_node(edge const& e, event_type const ev_type) {
  return ev_type == event_type::DEP ? e.from_ : e.to_;
}

void add_to_propagator(schedule& sched, ris::DelayMessage const* msg,
                       delay_propagator& propagator) {
  auto const id = msg->trip_id();
  auto const updates = get_updates(sched, msg->type() == ris::DelayType_Is
                                              ? delay_info::reason::IS
                                              : delay_info::reason::FORECAST,
                                   msg->events());
  auto const trp = *get_trip(sched, id->station_id()->str(), id->service_num(),
                             id->schedule_time());
  auto const lcon_idx = trp.lcon_idx_;

  // For each edge of the trip:
  for (auto& trp_e : *trp.edges_) {
    auto& e = *trp_e.get_edge();
    auto& lcon = e.m_.route_edge_.conns_[lcon_idx];

    // For each event of the edge / light connection:
    for (auto ev_type : {event_type::DEP, event_type::ARR}) {
      auto const route_node = get_route_node(e, ev_type);
      auto& ev_time = get_event_time(lcon, ev_type);

      // Try updates:
      for (auto const& upd : updates) {
        // Check whether station matches update station.
        if (upd.station_id_ != route_node->get_station()->id_) {
          continue;
        }

        // Get delay info from graph event.
        auto const graph_ev = ev_key(&e, lcon_idx, ev_type);
        auto di_it = sched.graph_to_delay_info_.find(graph_ev);

        // Check whether schedule time matches update message schedule time.
        auto const schedule_time = di_it != end(sched.graph_to_delay_info_)
                                       ? di_it->second->get_schedule_time()
                                       : ev_time;
        if (upd.schedule_time_ != schedule_time) {
          continue;
        }

        propagator.add_delay(ev_key(&e, lcon_idx, ev_type), upd.reason_,
                             upd.updated_time_);
      }
    }
  }
}

bool is_corrupt(ev_key const& k) {
  // Check conflict with earlier and later connection.
  auto const ev_time = get_event_time(*k.lcon(), k.ev_type_);
  auto const next = k.route_edge_->get_next_valid_lcon(k.lcon(), 1);
  auto const prev = k.route_edge_->get_prev_valid_lcon(k.lcon(), 1);

  if ((next && get_event_time(*next, k.ev_type_) <= ev_time) ||
      (prev && get_event_time(*prev, k.ev_type_) >= ev_time)) {
    return true;
  }

  // Check conflict with previous and next events.
  switch (k.ev_type_) {
    case event_type::ARR: {
      auto const dep_time = get_time(k.get_opposite());
      auto const arr_time = get_time(k);
      if (dep_time > arr_time) {
        return true;
      }

      bool valid = true;
      for_each_departure(k, [&](ev_key const& dep) {
        valid = valid && get_time(dep) >= arr_time;
      });
      return !valid;
    }

    case event_type::DEP: {
      auto const dep_time = get_time(k);
      auto const arr_time = get_time(k.get_opposite());
      if (dep_time > arr_time) {
        return true;
      }

      bool valid = true;
      for_each_arrival(k, [&](ev_key const& arr) {
        valid = valid && get_time(arr) <= dep_time;
      });
      return !valid;
    }

    default: return true;
  }
}

void disable_route_layer(ev_key const& k) {
  std::set<edge const*> visited;
  std::queue<edge const*> q;
  while (!q.empty()) {
    auto const e = q.front();
    q.pop();

    for (auto const& in : e->from_->incoming_edges_) {
      if (in->empty()) {
        continue;
      }

      auto res = visited.insert(in);
      if (res.second) {
        q.push(in);
      }
    }

    for (auto const& out : e->to_->edges_) {
      if (out.empty()) {
        continue;
      }

      auto res = visited.insert(&out);
      if (res.second) {
        q.push(&out);
      }
    }
  }

  for (auto const& e : visited) {
    const_cast<light_connection&>(e->m_.route_edge_.conns_[k.lcon_idx_])
        .valid_ = false;
  }
}

po::options_description rt::desc() {
  po::options_description desc("RT Module");
  return desc;
}

void rt::init(motis::module::registry& reg) {
  reg.subscribe("/ris/messages", [](msg_ptr const& msg) -> msg_ptr {
    auto& sched = get_schedule();

    // Parse message and add updates to propagator.
    auto propagator = delay_propagator(sched);
    for (auto const& m : *motis_content(RISBatch, msg)->messages()) {
      auto const& nested = m->message_nested_root();
      if (nested->content_type() != ris::MessageUnion_DelayMessage) {
        continue;
      }

      try {
        add_to_propagator(sched, reinterpret_cast<ris::DelayMessage const*>(
                                     nested->content()),
                          propagator);
      } catch (...) {
        continue;
      }
    }

    propagator.propagate();

    // Update graph.
    for (auto const& ev : propagator.events()) {
      auto const& di = ev.second;
      auto const& k = di->get_ev_key();

      auto& event_time =
          k.ev_type_ == event_type::DEP ? k.lcon()->d_time_ : k.lcon()->a_time_;
      const_cast<time&>(event_time) = di->get_current_time();  // NOLINT
    }

    // Check for graph corruption and revert if necessary.
    std::vector<delay_info*> checked_events;
    for (auto const& ev : propagator.events()) {
      auto const& di = ev.second;
      auto const& k = di->get_ev_key();

      if (is_corrupt(k)) {
        disable_route_layer(k);
      } else {
        checked_events.push_back(ev.second);
      }
    }

    // Generate shifted nodes
    // and update the delay info map.
    shifted_nodes_msg_builder shifted_nodes(sched);
    for (auto const& di : checked_events) {
      auto const& k = di->get_ev_key();

      shifted_nodes.add_shifted_node(*di);

      map_get_or_create(sched.graph_to_delay_info_, k, [&]() {
        sched.delay_mem_.push_back(std::make_unique<delay_info>(*di));
        return sched.delay_mem_.back().get();
      })->update(*di);
    }

    if (!shifted_nodes.empty()) {
      motis_publish(shifted_nodes.finish());
    }

    return nullptr;
  });
}

}  // namespace rt
}  // namespace motis
