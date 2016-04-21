#pragma once

#include "motis/core/schedule/edges.h"
#include "motis/core/schedule/schedule.h"
#include "motis/core/schedule/trip.h"
#include "motis/core/access/connection_access.h"
#include "motis/core/access/edge_access.h"

namespace motis {
namespace access {

class trip_section {
public:
  trip_section(trip const* t, int const index)
      : trip_(t), index_(index), edge_(t->edges_->at(index)) {}

  int index() const { return index_; }

  light_connection const& lcon() const {
    return get_lcon(edge_, trip_->lcon_idx_);
  }

  connection const& fcon() const { return *lcon().full_con_; }

  connection_info const& info(schedule const& sched) const {
    return get_connection_info(sched, lcon(), trip_);
  }

  station const& from_station(schedule const& sched) const {
    return get_station(sched, edge_->from_);
  }

  station const& to_station(schedule const& sched) const {
    return get_station(sched, edge_->to_);
  }

private:
  station const& get_station(schedule const& sched, node const* n) const {
    return *sched.stations_[n->get_station()->id_];
  }

  trip const* trip_;
  int index_;
  edge const* edge_;
};

}  // namespace access
}  // namespace motis
