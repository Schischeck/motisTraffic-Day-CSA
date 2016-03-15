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
    assert(index < edge_->_m._route_edge._conns.size());
  }

  int index() const { return index_; }

  light_connection const& lcon() const {
    return edge_->_m._route_edge._conns[trip_->lcon_idx];
  }

private:
  trip const* trip_;
  int index_;
  edge const* edge_;
};

}  // namespace access
}  // namespace motis
