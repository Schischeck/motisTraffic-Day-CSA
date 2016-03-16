#pragma once

#include "motis/core/schedule/trip.h"
#include "motis/core/schedule/edges.h"

namespace motis {
namespace access {

class trip_section {
public:
  trip_section(trip const* t, int const index)
      : trip_(t), index_(index), edge_(t->edges->at(index)) {
    assert(edge_->type() == edge::ROUTE_EDGE);
    assert(trip_->lcon_idx < edge_->_m._route_edge._conns.size());
  }

  int index() const { return index_; }

  light_connection const& lcon() const {
    return edge_->_m._route_edge._conns[trip_->lcon_idx];
  }

  connection const& fcon() const {
    return *lcon()._full_con;
  }

  connection_info const& info(schedule const& sched) const {
    auto const& trips = *sched.merged_trips[lcon().trips];
    if(trips.size() == 1) {
      return *fcon().con_info;
    }

    auto const it = std::find(begin(trips), end(trips), trip_);
    assert(it != end(trips));
    auto const pos = std::distance(begin(trips), it);

    auto info = fcon().con_info;
    for(int i = 0; i < pos; ++i) {
      info = info->merged_with;
    }
    return *info;
  }

  station const& from_station(schedule const& sched) const {
    return get_station(sched, edge_->_from);
  }

  station const& to_station(schedule const& sched) const {
    return get_station(sched, edge_->_to);
  }

private:
  station const& get_station(schedule const& sched, node const* n) const {
    return *sched.stations[n->get_station()->_id];
  }

  trip const* trip_;
  int index_;
  edge const* edge_;
};

}  // namespace access
}  // namespace motis
