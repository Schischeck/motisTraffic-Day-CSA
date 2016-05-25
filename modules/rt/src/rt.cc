#include "motis/rt/rt.h"

#include <iostream>

#include "boost/program_options.hpp"

#include "motis/core/access/realtime_access.h"
#include "motis/core/access/station_access.h"
#include "motis/core/access/time_access.h"
#include "motis/core/access/trip_access.h"
#include "motis/module/context/get_schedule.h"
#include "motis/loader/util.h"

namespace po = boost::program_options;
using namespace flatbuffers;
using namespace motis::module;
using motis::ris::RISBatch;

namespace motis {
namespace rt {

struct update {
  update() = default;
  update(unsigned station_id, time schedule_time, time new_time)
      : station_id_(station_id),
        schedule_time_(schedule_time),
        updated_time_(new_time) {}

  schedule_event sched_ev(primary_trip_id const& id, event_type type) const {
    return {id, station_id_, type, schedule_time_};
  }

  unsigned station_id_;
  time schedule_time_, updated_time_;
};

std::vector<update> get_updates(
    schedule const& sched, Vector<Offset<ris::UpdatedEvent>> const* events) {
  std::vector<update> updates;

  for (auto const& ev : *events) {
    try {
      auto const station_id = ev->base()->station_id()->str();
      auto const station_node = get_station_node(sched, station_id)->id_;
      auto const time = unix_to_motistime(sched, ev->base()->schedule_time());
      auto const upd_time = unix_to_motistime(sched, ev->updated_time());
      if (time != INVALID_TIME && upd_time != INVALID_TIME) {
        updates.emplace_back(station_node, time, upd_time);
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

unsigned get_station_id(edge const& e, event_type const ev_type) {
  return (ev_type == event_type::DEP ? e.from_ : e.to_)->get_station()->id_;
}

void update_event_time(schedule& sched, light_connection& lcon,
                       primary_trip_id const& /* trp */, event_type ev_type,
                       schedule_event const& /* sched_ev */,
                       graph_event const& /* graph_ev */, time updated_time) {
  (ev_type == event_type::DEP ? lcon.d_time_ : lcon.a_time_) = updated_time;
  // TODO(Felix Guendling) update maps in sched
  (void)(sched);
}

void handle_delay_message(schedule& sched, ris::DelayMessage const* msg) {
  auto const id = msg->trip_id();
  auto const updates = get_updates(sched, msg->events());
  auto const trp = *get_trip(sched, id->station_id()->str(), id->service_num(),
                             id->schedule_time());
  auto const trp_id = trp.id_.primary_;
  auto const lcon_idx = trp.lcon_idx_;

  // For each edge of the trip
  for (auto& trp_e : *trp.edges_) {
    auto& e = *trp_e.get_edge();
    auto& lcon = e.m_.route_edge_.conns_[lcon_idx];

    // For each event of the edge / light connection
    for (auto ev_type : {event_type::DEP, event_type::ARR}) {
      auto const station = get_station_id(e, ev_type);
      auto const ev_time = get_event_time(lcon, ev_type);

      // Check whether an RT update matches this event:
      for (auto const& upd : updates) {
        if (upd.station_id_ != station) {
          continue;
        }

        auto const graph_ev = graph_event(trp_id, station, ev_type, ev_time);
        auto const sched_time = get_schedule_time(sched, graph_ev);
        if (sched_time != upd.schedule_time_) {
          continue;
        }

        auto const sched_ev =
            schedule_event(trp_id, station, ev_type, sched_time);
        update_event_time(sched, lcon, trp_id, ev_type, sched_ev, graph_ev,
                          upd.updated_time_);
      }
    }
  }
}

po::options_description rt::desc() {
  po::options_description desc("RT Module");
  return desc;
}

void rt::init(motis::module::registry& reg) {
  reg.subscribe("/ris/messages", [](msg_ptr const& msg) -> msg_ptr {
    auto& sched = get_schedule();
    for (auto const& m : *motis_content(RISBatch, msg)->messages()) {
      auto const& nested = m->message_nested_root();
      if (nested->content_type() != ris::MessageUnion_DelayMessage) {
        continue;
      }

      auto const& delay_msg =
          reinterpret_cast<ris::DelayMessage const*>(nested->content());
      handle_delay_message(sched, delay_msg);
    }
    return nullptr;
  });
}

}  // namespace rt
}  // namespace motis
