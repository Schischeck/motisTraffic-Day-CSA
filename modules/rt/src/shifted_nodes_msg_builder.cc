#include "motis/rt/shifted_nodes_msg_builder.h"

#include "motis/core/access/edge_access.h"
#include "motis/core/access/time_access.h"
#include "motis/core/conv/event_type_conv.h"
#include "motis/core/conv/timestamp_reason_conv.h"

using namespace motis::module;

namespace motis {
namespace rt {

shifted_nodes_msg_builder::shifted_nodes_msg_builder(schedule const& sched)
    : sched_(sched) {}

void shifted_nodes_msg_builder::add(delay_info const* di) {
  delays_.insert(di);
}

void shifted_nodes_msg_builder::build_shifted_node(delay_info const* di) {
  auto const& k = di->get_ev_key();

  auto const trp =
      sched_.merged_trips_[get_lcon(k.route_edge_, k.lcon_idx_).trips_]
          ->at(0)
          ->id_;
  auto const primary = trp.primary_;
  auto const secondary = trp.secondary_;

  nodes_.push_back(CreateShiftedNode(
      fbb_,
      CreateTripId(
          fbb_,
          fbb_.CreateString(sched_.stations_.at(primary.station_id_)->eva_nr_),
          primary.train_nr_, motis_to_unixtime(sched_, primary.time_),
          fbb_.CreateString(
              sched_.stations_.at(secondary.target_station_id_)->eva_nr_),
          motis_to_unixtime(sched_, secondary.target_time_),
          fbb_.CreateString(secondary.line_id_)),
      fbb_.CreateString(sched_.stations_.at(k.get_station_idx())->eva_nr_),
      motis_to_unixtime(sched_, di->get_schedule_time()), to_fbs(k.ev_type_),
      motis_to_unixtime(sched_, di->get_current_time()),
      to_fbs(di->get_reason()), false));
}

msg_ptr shifted_nodes_msg_builder::finish() {
  for (auto const& di : delays_) {
    build_shifted_node(di);
  }

  fbb_.create_and_finish(
      MsgContent_RtUpdate,
      CreateRtUpdate(fbb_, fbb_.CreateVector(nodes_)).Union(), "/rt/update",
      DestinationType_Topic);
  return make_msg(fbb_);
}

bool shifted_nodes_msg_builder::empty() const { return delays_.empty(); }

std::size_t shifted_nodes_msg_builder::size() const { return delays_.size(); }

}  // namespace rt
}  // namespace motis
