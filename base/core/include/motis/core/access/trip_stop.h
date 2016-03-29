#pragma once

#include "motis/core/access/connection_access.h"
#include "motis/core/access/edge_access.h"
#include "motis/core/schedule/trip.h"
#include "motis/core/schedule/edges.h"

namespace motis {
namespace access {

class trip_stop {
public:
  trip_stop(trip const* t, int const index) : trip_(t), index_(index) {
    if (index == static_cast<int>(trip_->edges->size())) {
      node_ = trip_->edges->back()->_to;
    } else {
      node_ = trip_->edges->at(index)->_from;
    }
    assert(node_->is_route_node());
  }

  int index() const { return index_; }

  bool has_arrival() const { return index_ > 0; }
  bool has_departure() const {
    return index_ < static_cast<int>(trip_->edges->size());
  }

  light_connection const& arr_lcon() const {
    return get_lcon(trip_->edges->at(index_ - 1), trip_->lcon_idx);
  }

  light_connection const& dep_lcon() const {
    return get_lcon(trip_->edges->at(index_), trip_->lcon_idx);
  }

  connection_info const& arr_info(schedule const& sched) const {
    return get_connection_info(sched, arr_lcon(), trip_);
  }

  connection_info const& dep_info(schedule const& sched) const {
    return get_connection_info(sched, dep_lcon(), trip_);
  }

  station const& get_station(schedule const& sched) const {
    return *sched.stations[node_->get_station()->_id];
  }

private:
  trip const* trip_;
  int index_;
  node const* node_;
};

}  // namespace access
}  // namespace motis
