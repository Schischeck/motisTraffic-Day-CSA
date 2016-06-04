#include "motis/rt/rt.h"

#include <iostream>

#include "boost/program_options.hpp"

#include "motis/core/access/realtime_access.h"
#include "motis/core/access/station_access.h"
#include "motis/core/access/time_access.h"
#include "motis/core/access/trip_access.h"
#include "motis/module/context/get_schedule.h"
#include "motis/module/context/motis_publish.h"
#include "motis/loader/util.h"
#include "motis/rt/delay_propagator.h"

namespace po = boost::program_options;
using namespace flatbuffers;
using namespace motis::module;
using motis::ris::RISBatch;

namespace motis {
namespace rt {

struct shifted_node {
  shifted_node() = default;
  shifted_node(primary_trip_id trp,  // NOLINT
               uint32_t station_idx, time schedule_time, event_type ev_type,
               time updated_time, delay_info::reason reason, bool canceled)
      : trp_(trp),
        station_idx_(station_idx),
        schedule_time_(schedule_time),
        ev_type_(ev_type),
        updated_time_(updated_time),
        reason_(reason),
        canceled_(canceled) {}

  primary_trip_id trp_;
  uint32_t station_idx_;
  time schedule_time_;
  event_type ev_type_;

  time updated_time_;
  delay_info::reason reason_;
  bool canceled_;
};

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

msg_ptr shifted_nodes_to_msg(schedule const& sched,
                             std::vector<shifted_node> const& shifted_nodes) {
  message_creator fbb;
  std::vector<Offset<ShiftedNode>> nodes;
  for (auto const& n : shifted_nodes) {
    nodes.push_back(CreateShiftedNode(
        fbb,
        CreateTripId(
            fbb,
            fbb.CreateString(sched.stations_.at(n.trp_.station_id_)->eva_nr_),
            n.trp_.train_nr_, motis_to_unixtime(sched, n.trp_.time_),
            fbb.CreateString(""), 0, EventType_Departure, fbb.CreateString("")),
        fbb.CreateString(sched.stations_.at(n.station_idx_)->eva_nr_),
        motis_to_unixtime(sched, n.schedule_time_),
        n.ev_type_ == event_type::DEP ? EventType_DEPARTURE : EventType_ARRIVAL,
        motis_to_unixtime(sched, n.updated_time_),
        n.reason_ == delay_info::reason::IS ? TimestampReason_IS
                                            : TimestampReason_FORECAST,
        false));
  }
  fbb.create_and_finish(MsgContent_RtUpdate,
                        CreateRtUpdate(fbb, fbb.CreateVector(nodes)).Union(),
                        "/rt/update", DestinationType_Topic);
  return make_msg(fbb);
}

po::options_description rt::desc() {
  po::options_description desc("RT Module");
  return desc;
}

void rt::init(motis::module::registry& reg) {
  reg.subscribe("/ris/messages", [](msg_ptr const& msg) -> msg_ptr {
    auto& sched = get_schedule();

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

    std::vector<shifted_node> shifted_nodes;
    for (auto const& ev : propagator.events()) {
      auto const& di = ev.second;
      auto const& k = di->get_ev_key();

      auto const trp =
          sched.merged_trips_[get_lcon(k.route_edge_, k.lcon_idx_).trips_]
              ->at(0)
              ->id_.primary_;

      shifted_nodes.emplace_back(
          trp, k.get_station_idx(), di->get_schedule_time(), k.ev_type_,
          di->get_current_time(), di->get_reason(), false);

      map_get_or_create(sched.graph_to_delay_info_, k, [&]() {
        sched.delay_mem_.push_back(std::make_unique<delay_info>(*di));
        return sched.delay_mem_.back().get();
      })->update(*di);

      auto& event_time =
          k.ev_type_ == event_type::DEP ? k.lcon()->d_time_ : k.lcon()->a_time_;
      const_cast<time&>(event_time) = di->get_current_time();  // NOLINT
    }

    if (!shifted_nodes.empty()) {
      motis_publish(shifted_nodes_to_msg(sched, shifted_nodes));
    }

    return nullptr;
  });
}

}  // namespace rt
}  // namespace motis
