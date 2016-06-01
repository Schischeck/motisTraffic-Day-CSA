#include "motis/rt/rt.h"

#include <iostream>

#include "boost/program_options.hpp"

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
  update(unsigned station_id, delay_info::reason reason, time schedule_time,
         time new_time)
      : station_id_(station_id),
        reason_(reason),
        schedule_time_(schedule_time),
        updated_time_(new_time) {}

  schedule_event sched_ev(primary_trip_id const& id, event_type type) const {
    return {id, station_id_, type, schedule_time_};
  }

  unsigned station_id_;
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

void handle_delay_message(schedule& sched, ris::DelayMessage const* msg) {
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
                                       ? di_it->second->schedule_time_
                                       : ev_time;
        if (upd.schedule_time_ != schedule_time) {
          continue;
        }

        delay_info* di = nullptr;
        if (di_it == end(sched.graph_to_delay_info_)) {
          // Create new delay info.
          sched.delay_infos_.emplace_back(new delay_info(schedule_time));
          di = sched.delay_infos_.back().get();

          auto const sched_ev =
              schedule_event(trp_id, route_node->get_station()->id_, ev_type,
                             di->schedule_time_);
          sched.schedule_to_delay_info_[sched_ev] = di;
          sched.graph_to_delay_info_[graph_ev] = di;
        } else {
          di = di_it->second;
        }

        // Update event.
        std::cout << upd.reason_ << " "
                  << motis_to_unixtime(sched, upd.updated_time_) << "\n";
        di->set(upd.reason_, upd.updated_time_);
        ev_time = di->get_current_time();
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
