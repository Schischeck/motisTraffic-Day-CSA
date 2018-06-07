#pragma once

#include <set>
#include <tuple>

#include "utl/to_set.h"

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
        transfers_(
            std::max(0, static_cast<int>(std::count_if(
                            std::begin(*c->stops()), std::end(*c->stops()),
                            [](Stop const* s) { return s->exit(); })) -
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

  template <int AlphaNominator = 1, int AlphaDenominator = 2>
  inline bool dominates(journey_meta_data const& o) const {
    static constexpr auto const alpha =
        static_cast<double>(AlphaNominator) / AlphaDenominator;

    bool could_dominate = false;

    if (transfers_ > o.transfers_) {
      return false;
    }
    could_dominate = could_dominate || transfers_ < o.transfers_;

    auto const tt_a = (get_arrival_time() - get_departure_time()) / 60;
    auto const tt_b = (o.get_arrival_time() - o.get_departure_time()) / 60;
    auto const tt_ratio = static_cast<double>(tt_a) / tt_b;
    auto const d_dep = std::abs(get_departure_time() - o.get_departure_time());
    auto const d_arr = std::abs(get_arrival_time() - o.get_arrival_time());
    auto const dist = std::min(d_dep, d_arr) / 60;

    auto const smaller = [&]() {
      return tt_a + alpha * tt_ratio * dist < tt_b;
    };
    auto const greater = [&]() {
      return tt_a + alpha * tt_ratio * dist > tt_b;
    };

    if (greater()) {
      return false;
    }
    could_dominate = could_dominate || smaller();

    return could_dominate;
  }

  Connection const* c_;
  unsigned duration_;
  unsigned transfers_;
};

struct response {
  explicit response(routing::RoutingResponse const* r)
      : connections_{utl::to_set(
            *r->connections(),
            [](Connection const* c) { return journey_meta_data(c); })},
        r_{r} {}
  std::set<journey_meta_data> connections_;
  routing::RoutingResponse const* r_;
};

}  // namespace comparator
}  // namespace eval
}  // namespace motis
