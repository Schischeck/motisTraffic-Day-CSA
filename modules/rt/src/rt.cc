#include "motis/rt/rt.h"

#include <iostream>

#include "boost/program_options.hpp"

#include "motis/core/access/station_access.h"
#include "motis/core/access/time_access.h"
#include "motis/core/access/trip_access.h"
#include "motis/module/context/get_schedule.h"
#include "motis/module/context/motis_publish.h"
#include "motis/loader/util.h"

namespace po = boost::program_options;
using namespace flatbuffers;
using namespace motis::module;
using motis::ris::RISBatch;

namespace motis {
namespace rt {

struct shifted_node {
  shifted_node() = default;
  shifted_node(primary_trip_id trp, uint32_t station_idx, time schedule_time,  // NOLINT
               event_type ev_type, time updated_time, delay_info::reason reason,
               bool canceled)
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

bool applicable(schedule const&) {
  // TODO(Felix Guendling):
  // check whether update can be applied to the search graph without losing
  // important properties.
  return true;
}

void handle_delay_message(schedule& sched, ris::DelayMessage const* msg,
                          std::vector<shifted_node>& shifted_nodes) {

  auto const id = msg->trip_id();
  auto const updates = get_updates(sched, msg->type() == ris::DelayType_Is
                                              ? delay_info::reason::IS
                                              : delay_info::reason::FORECAST,
                                   msg->events());
  auto const trp = *get_trip(sched, id->station_id()->str(), id->service_num(),
                             id->schedule_time());
  auto const trp_id = trp.id_.primary_;
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
        auto const graph_ev = graph_event(route_node, lcon_idx, ev_type);
        auto di_it = sched.graph_to_delay_info_.find(graph_ev);

        // Check whether schedule time matches update message schedule time.
        auto const schedule_time = di_it != end(sched.graph_to_delay_info_)
                                       ? di_it->second->get_schedule_time()
                                       : ev_time;
        if (upd.schedule_time_ != schedule_time) {
          continue;
        }

        // Check whether update is applicable.
        if (!applicable(sched)) {
          continue;
        }

        delay_info* di = nullptr;
        if (di_it == end(sched.graph_to_delay_info_)) {
          // Create new delay info.
          sched.delay_infos_.emplace_back(new delay_info(schedule_time));
          di = sched.delay_infos_.back().get();

          auto const sched_ev =
              schedule_event(trp_id, route_node->get_station()->id_, ev_type,
                             di->get_schedule_time());
          sched.schedule_to_delay_info_[sched_ev] = di;
          sched.graph_to_delay_info_[graph_ev] = di;
        } else {
          di = di_it->second;
        }

        // Update event.
        di->set(upd.reason_, upd.updated_time_);
        ev_time = di->get_current_time();

        // Store update for broadcast.
        shifted_nodes.emplace_back(trp_id, upd.station_id_, schedule_time,
                                   ev_type, di->get_current_time(),
                                   di->get_reason(), false);
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
    std::vector<shifted_node> shifted_nodes;

    auto& sched = get_schedule();
    for (auto const& m : *motis_content(RISBatch, msg)->messages()) {
      auto const& nested = m->message_nested_root();
      if (nested->content_type() != ris::MessageUnion_DelayMessage) {
        continue;
      }

      auto const& delay_msg =
          reinterpret_cast<ris::DelayMessage const*>(nested->content());

      try {
        handle_delay_message(sched, delay_msg, shifted_nodes);
      } catch (...) {
        continue;
      }
    }

    if (!shifted_nodes.empty()) {
      motis_publish(shifted_nodes_to_msg(sched, shifted_nodes));
    }

    return nullptr;
  });
}

}  // namespace rt
}  // namespace motis
