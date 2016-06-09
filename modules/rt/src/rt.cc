#include "motis/rt/rt.h"

#include <queue>

#include "boost/program_options.hpp"

#include "motis/core/common/logging.h"
#include "motis/core/access/edge_access.h"
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
using namespace motis::logging;
using motis::ris::RISBatch;

namespace motis {
namespace rt {

struct update {
  update() = default;
  update(uint32_t station_id, time schedule_time, event_type ev_type,
         delay_info::reason reason, time new_time)
      : station_id_(station_id),
        sched_time_(schedule_time),
        ev_type_(ev_type),
        reason_(reason),
        upd_time_(new_time) {}

  friend bool operator<(update const& a, update const& b) {
    return std::tie(a.station_id_, a.reason_, a.sched_time_, a.upd_time_) <
           std::tie(b.station_id_, b.reason_, b.sched_time_, b.upd_time_);
  }

  friend bool operator==(update const& a, update const& b) {
    return std::tie(a.station_id_, a.reason_, a.sched_time_, a.upd_time_) ==
           std::tie(b.station_id_, b.reason_, b.sched_time_, b.upd_time_);
  }

  uint32_t station_id_;
  time sched_time_;
  event_type ev_type_;

  delay_info::reason reason_;
  time upd_time_;
};

std::vector<update> get_updates(schedule const& sched,
                                delay_info::reason const reason,
                                Vector<Offset<ris::UpdatedEvent>> const* events,
                                statistics& stats) {
  std::vector<update> updates;

  for (auto const& ev : *events) {
    try {
      auto const station_id = ev->base()->station_id()->str();
      auto const station_node = get_station_node(sched, station_id)->id_;
      auto const time = unix_to_motistime(sched, ev->base()->schedule_time());
      auto const upd_time = unix_to_motistime(sched, ev->updated_time());
      if (time != INVALID_TIME && upd_time != INVALID_TIME) {
        updates.emplace_back(
            station_node, time,
            ev->base()->type() == motis::ris::EventType_Departure
                ? event_type::DEP
                : event_type::ARR,
            reason, upd_time);
      } else {
        ++stats.ev_invalid_time_;
      }
    } catch (...) {
      ++stats.ev_station_not_found_;
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

void rt::add_to_propagator(schedule const& sched,
                           ris::DelayMessage const* msg) {
  stats_.total_evs_ += msg->events()->size();

  // TODO(Felix Guendling) remove this when additional trains are supported
  if (msg->trip_id()->trip_type() != ris::IdEventType_Schedule) {
    stats_.additional_not_found_ += msg->events()->size();
    return;
  }

  // Retrieve trip.
  // - first try: exact match of first departure
  // - second try: match of first departure with train number set to 0
  trip const* trp;
  auto const id = msg->trip_id();
  trp = find_trip(sched, id->station_id()->str(), id->service_num(),
                  id->schedule_time());
  if (trp == nullptr) {
    stats_.ev_exact_trp_not_found_ += msg->events()->size();
    trp = find_trip(sched, id->station_id()->str(), 0, id->schedule_time());
  }
  if (trp == nullptr) {
    stats_.ev_trp_not_found_ += msg->events()->size();
    return;
  }

  // Translate external identifiers to internal identifiers.
  auto const updates = get_updates(sched, msg->type() == ris::DelayType_Is
                                              ? delay_info::reason::IS
                                              : delay_info::reason::FORECAST,
                                   msg->events(), stats_);
  auto const lcon_idx = trp->lcon_idx_;
  stats_.total_updates_ += updates.size();

  // For each edge of the trip:
  for (auto& trp_e : *trp->edges_) {
    auto& e = *trp_e.get_edge();
    auto& lcon = e.m_.route_edge_.conns_[lcon_idx];

    // For each event of the edge / light connection:
    for (auto ev_type : {event_type::DEP, event_type::ARR}) {
      auto const route_node = get_route_node(e, ev_type);
      auto& ev_time = get_event_time(lcon, ev_type);

      // Try updates:
      for (auto const& upd : updates) {
        // Check whether station matches update station.
        if (upd.ev_type_ != ev_type ||
            upd.station_id_ != route_node->get_station()->id_) {
          continue;
        }

        // Get delay info from graph event.
        auto const graph_ev = ev_key(&e, lcon_idx, ev_type);
        auto di_it = sched.graph_to_delay_info_.find(graph_ev);

        // Check whether schedule time matches update message schedule time.
        auto const schedule_time = di_it != end(sched.graph_to_delay_info_)
                                       ? di_it->second->get_schedule_time()
                                       : ev_time;
        auto const diff =
            std::abs(static_cast<int>(upd.sched_time_) - schedule_time);
        if (diff != 0) {
          ++stats_.update_mismatch_sched_time_;
        }
        if (diff > 5) {
          ++stats_.diff_gt_5_;
          if (diff > 10) {
            ++stats_.diff_gt_10_;
          }
          if (diff > 30) {
            ++stats_.diff_gt_30_;
          }
          continue;
        }

        propagator_->add_delay(ev_key(&e, lcon_idx, ev_type), upd.reason_,
                               upd.upd_time_);
        ++stats_.found_updates_;
      }
    }
  }
}

bool is_corrupt(ev_key const& k, statistics& stats) {
  // Check conflict with earlier and later connection.
  auto const ev_time = get_event_time(*k.lcon(), k.ev_type_);
  auto const next = k.route_edge_->get_next_valid_lcon(k.lcon(), 1);
  auto const prev = k.route_edge_->get_prev_valid_lcon(k.lcon(), 1);

  if ((next && get_event_time(*next, k.ev_type_) <= ev_time) ||
      (prev && get_event_time(*prev, k.ev_type_) >= ev_time)) {
    ++stats.route_overtake_;
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
  q.push(k.route_edge_);
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
    auto const& con = e->m_.route_edge_.conns_[k.lcon_idx_];
    auto& mutable_con = const_cast<light_connection&>(con);  // NOLINT
    mutable_con.valid_ = false;
  }
}

rt::rt() = default;

rt::~rt() = default;

po::options_description rt::desc() {
  po::options_description desc("RT Module");
  return desc;
}

void rt::init(motis::module::registry& reg) {
  reg.subscribe("/ris/messages", [&](msg_ptr const& msg) -> msg_ptr {
    auto& sched = get_schedule();
    auto msgs = motis_content(RISBatch, msg)->messages();
    LOG(info) << "rt received " << msgs->size() << " ris messages";

    if (!propagator_) {
      LOG(info) << "initializing propagator";
      propagator_ = std::make_unique<delay_propagator>(sched);
      stats_ = statistics();
    }

    // Parse message and add updates to propagator.
    for (auto const& m : *msgs) {
      auto const& nested = m->message_nested_root();
      if (nested->content_type() != ris::MessageUnion_DelayMessage) {
        continue;
      }

      auto const content =
          reinterpret_cast<ris::DelayMessage const*>(nested->content());
      try {
        add_to_propagator(sched, content);
      } catch (...) {
        continue;
      }
    }

    LOG(info) << "rt propagator: " << propagator_->events().size() << " events";
    return nullptr;
  });

  reg.subscribe("/ris/system_time_changed", [&](msg_ptr const&) -> msg_ptr {
    scoped_timer timer("rt update");
    manual_timer graph_update("graph update");

    auto& sched = get_schedule();

    if (!propagator_) {
      LOG(info) << "rt no updates - ignoring system time change";
      return nullptr;
    }

    LOG(info) << "rt propagating " << propagator_->events().size() << " events";
    propagator_->propagate();
    LOG(info) << "rt total changes " << propagator_->events().size();

    // Update graph.
    for (auto const& ev : propagator_->events()) {
      auto const& di = ev.second;
      auto const& k = di->get_ev_key();

      auto& event_time =
          k.ev_type_ == event_type::DEP ? k.lcon()->d_time_ : k.lcon()->a_time_;
      const_cast<time&>(event_time) = di->get_current_time();  // NOLINT
    }

    // Check for graph corruption and revert if necessary.
    std::vector<delay_info*> checked_events;
    for (auto const& ev : propagator_->events()) {
      auto const& di = ev.second;
      auto const& k = di->get_ev_key();

      if (!k.lcon()->valid_) {
        continue;
      } else if (is_corrupt(k, stats_)) {
        ++stats_.disabled_routes_;
        disable_route_layer(k);
      } else {
        checked_events.push_back(ev.second);
      }
    }

    stats_.propagated_updates_ = propagator_->events().size();
    stats_.graph_updates_ = checked_events.size();

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

    graph_update.stop_and_print();

    if (!shifted_nodes.empty()) {
      motis_publish(shifted_nodes.finish());
    }

    manual_timer lb_update("lower bound graph update");
    sched.lower_bounds_ = constant_graph(sched.station_nodes_);
    lb_update.stop_and_print();

    std::cout << stats_ << std::endl;

    propagator_.reset();

    return nullptr;
  });
}

}  // namespace rt
}  // namespace motis
