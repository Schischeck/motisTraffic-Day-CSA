#include "../include/motis/rt/shifted_nodes_msg_builder.h"

#include "motis/core/access/edge_access.h"
#include "motis/core/access/time_access.h"

using namespace motis::module;
namespace fbs = flatbuffers;

namespace motis {
namespace rt {

shifted_nodes_msg_builder::shifted_nodes_msg_builder(schedule const& sched)
    : sched_(sched) {}

void shifted_nodes_msg_builder::add_shifted_node(delay_info const& di) {
  auto const& k = di.get_ev_key();

  auto const trp =
      sched_.merged_trips_[get_lcon(k.route_edge_, k.lcon_idx_).trips_]
          ->at(0)
          ->id_.primary_;

  nodes_.push_back(CreateShiftedNode(
      fbb_,
      CreateTripId(
          fbb_,
          fbb_.CreateString(sched_.stations_.at(trp.station_id_)->eva_nr_),
          trp.train_nr_, motis_to_unixtime(sched_, trp.time_),
          fbb_.CreateString(""), 0, EventType_Departure, fbb_.CreateString("")),
      fbb_.CreateString(sched_.stations_.at(k.get_station_idx())->eva_nr_),
      motis_to_unixtime(sched_, di.get_schedule_time()),
      k.ev_type_ == event_type::DEP ? EventType_DEPARTURE : EventType_ARRIVAL,
      motis_to_unixtime(sched_, di.get_current_time()),
      di.get_reason() == delay_info::reason::IS ? TimestampReason_IS
                                                : TimestampReason_FORECAST,
      false));
}

msg_ptr shifted_nodes_msg_builder::finish() {
  fbb_.create_and_finish(
      MsgContent_RtUpdate,
      CreateRtUpdate(fbb_, fbb_.CreateVector(nodes_)).Union(), "/rt/update",
      DestinationType_Topic);
  return make_msg(fbb_);
}

bool shifted_nodes_msg_builder::empty() const { return nodes_.empty(); }

}  // namespace rt
}  // namespace motis
