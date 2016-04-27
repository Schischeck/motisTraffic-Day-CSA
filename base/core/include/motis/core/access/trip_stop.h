#pragma once

#include "motis/core/schedule/edges.h"
#include "motis/core/schedule/trip.h"
#include "motis/core/access/connection_access.h"
#include "motis/core/access/edge_access.h"

namespace motis {
namespace access {

class trip_stop {
public:
  trip_stop(trip const* t, int const index) : trip_(t), index_(index) {
    if (index == static_cast<int>(trip_->edges_->size())) {
      node_ = trip_->edges_->back()->to_;
    } else {
      node_ = trip_->edges_->at(index)->from_;
    }
    assert(node_->is_route_node());
  }

  int index() const { return index_; }

  bool has_arrival() const { return index_ > 0; }
  bool has_departure() const {
    return index_ < static_cast<int>(trip_->edges_->size());
  }

  light_connection const& arr_lcon() const {
    return get_lcon(trip_->edges_->at(index_ - 1), trip_->lcon_idx_);
  }

  light_connection const& dep_lcon() const {
    return get_lcon(trip_->edges_->at(index_), trip_->lcon_idx_);
  }

  connection_info const& arr_info(schedule const& sched) const {
    return get_connection_info(sched, arr_lcon(), trip_);
  }

  connection_info const& dep_info(schedule const& sched) const {
    return get_connection_info(sched, dep_lcon(), trip_);
  }

  station const& get_station(schedule const& sched) const {
    return *sched.stations_[node_->get_station()->id_];
  }

  node const* get_route_node() const { return node_; }

private:
  trip const* trip_;
  int index_;
  node const* node_;
};

}  // namespace access
}  // namespace motis
