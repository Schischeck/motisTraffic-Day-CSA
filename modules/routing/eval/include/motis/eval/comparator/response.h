#pragma once

#include <set>
#include <tuple>

#include "motis/core/common/transform_to_set.h"

#include "motis/routing/label/criteria/transfers.h"
#include "motis/routing/label/criteria/travel_time.h"
#include "motis/routing/label/dominance.h"
#include "motis/routing/label/tie_breakers.h"

#include "motis/protocol/RoutingResponse_generated.h"

namespace motis {
namespace eval {
namespace comparator {

enum connection_accessors { DURATION, TRANSFERS, PRICE };

struct journey_meta_data {
  explicit journey_meta_data(Connection const* c)
      : c_(c),
        duration_(c->stops()->Get(c->stops()->Length() - 1)->arrival()->time() -
                  c->stops()->Get(0)->departure()->time()),
        transfers_(std::max(
            0u, std::count_if(std::begin(*c->stops()), std::end(*c->stops()),
                              [](Stop const* s) { return s->exit(); }) -
                    1)) {}

  inline friend bool operator==(journey_meta_data const& a,
                                journey_meta_data const& b) {
    return a.as_tuple() == b.as_tuple();
  }

  inline friend bool operator<(journey_meta_data const& a,
                               journey_meta_data const& b) {
    return a.as_tuple() < b.as_tuple();
  }

  inline time_t get_departure_time() const {
    return c_->stops()->Get(0)->departure()->time();
  }

  inline time_t get_arrival_time() const {
    return c_->stops()->Get(c_->stops()->Length() - 1)->arrival()->time();
  }

  inline std::tuple<int, int, int> as_tuple() const {
    return std::make_tuple(duration_, get_departure_time(), transfers_);
  }

  inline bool dominates(journey_meta_data const& o) const {
    bool could_dominate = false;

    if (transfers_ > o.transfers_) {
      return false;
    }
    could_dominate = could_dominate || transfers_ < o.transfers_;

    auto const dist_a = std::abs(get_departure_time() - o.get_departure_time());
    auto const dist_b = std::abs(get_arrival_time() - o.get_arrival_time());
    auto const dist = std::min(dist_a, dist_b) / 60;
    auto const travel_time_dominance =
        routing::travel_time_alpha_dominance::domination_info<int>(
            duration_, o.duration_, dist);
    if (travel_time_dominance.greater()) {
      return false;
    }
    could_dominate = could_dominate || travel_time_dominance.smaller();

    return could_dominate;
  }

  Connection const* c_;
  unsigned duration_;
  unsigned transfers_;
};

struct response {
  explicit response(routing::RoutingResponse const* r)
      : connections_(transform_to_set(
            *r->connections(),
            [](Connection const* c) { return journey_meta_data(c); })) {}
  std::set<journey_meta_data> connections_;
};

}  // namespace comparator
}  // namespace eval
}  // namespace motis
