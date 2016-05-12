#include "motis/rt/rt.h"

#include <iostream>

#include "boost/program_options.hpp"

#include "motis/protocol/Message_generated.h"

namespace p = std::placeholders;
namespace po = boost::program_options;
using namespace flatbuffers;
using namespace motis::module;
using motis::ris::RISBatch;

namespace motis {
namespace rt {

struct update {
  update() = default;
  update(unsigned station_id, time schedule_time)
      : station_id_(station_id), schedule_time_(schedule_time) {}
  unsigned station_id_;
  time schedule_time_;
};

std::vector<update> get_updates(
    schedule const& sched, Vector<Offset<ris::UpdatedEvent>> const* events) {
  return loader::transform_to_vec(
      events->begin(), events->end(), [&](ris::UpdatedEvent const* ev) {
        return update(get_station_node(sched, ev->base()->station_id()->str()),
                      unix_to_motistime(sched, ev->base()->schedule_time()));
      });
}

void handle_delay_message(schedule const& sched, ris::DelayMessage const* msg) {
  auto const id = msg->trip_id();
  auto const updates = get_updates(sched, msg->events());
  auto const trp = get_trip(sched, id->station_id()->str(), id->service_num(),
                            id->schedule_time());

  auto lcon_idx = trp->lcon_idx_;
  for (auto const& trp_e : *trp->edges_) {
    auto const& e = *trp_e.get_edge();
    for (auto const& upd : get_updates(sched, msg->events())) {
    }
    e.m_.route_edge_.conns_[lcon_idx];
  }
}

po::options_description rt::desc() {
  po::options_description desc("RT Module");
  return desc;
}

void rt::init(motis::module::registry& reg) {
  reg.subscribe("/ris/messages", [](msg_ptr const& msg) -> msg_ptr {
    auto const& sched = get_schedule();
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
