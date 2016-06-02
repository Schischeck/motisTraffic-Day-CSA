#pragma once

#include "motis/core/schedule/edges.h"

#include "motis/routing/late_connections_util.h"

namespace motis {
namespace routing {

struct late_connections {
  uint16_t night_penalty_, db_costs_;
  uint8_t visited_hotel_;
  enum hotel { NOT_VISITED, VISITED, FILTERED };
};

struct late_connections_initializer {
  template <typename Label, typename LowerBounds>
  static void init(Label& l, LowerBounds&) {
    l.night_penalty_ = 0;
    l.db_costs_ = 0;
    l.visited_hotel_ = late_connections::NOT_VISITED;
  }
};

struct late_connections_updater {
  template <typename Label, typename LowerBounds>
  static void update(Label& l, uint8_t t, edge_cost const& ec, LowerBounds&) {
    l.db_costs_ += ec.price_;
    if (t == edge::HOTEL_EDGE) {
      if (l.visited_hotel_ == late_connections::NOT_VISITED) {
        l.visited_hotel_ = late_connections::VISITED;
      }
    } else if (t == edge::PERIODIC_MUMO_EDGE /* taxi */ &&
               l.visited_hotel_ == late_connections::VISITED) {
      /* taxi after hotel not allowed */
      l.visited_hotel_ = late_connections::FILTERED;
    } else {
      l.night_penalty_ += late_connections_util::night_travel_duration(
          l.now_ - ec.time_, l.now_);
    }
  }
};

struct late_connections_dominance {
  template <typename Label>
  struct domination_info {
    domination_info(Label const& a, Label const& b)
        : greater_(a.visited_hotel_ != b.visited_hotel_ ||
                   a.db_costs_ > b.db_costs_ ||
                   a.night_penalty_ > b.night_penalty_),
          smaller_(a.visited_hotel_ == b.visited_hotel_ &&
                   a.db_costs_ <= b.db_costs_ &&
                   a.night_penalty_ <= b.night_penalty_ &&
                   (a.db_costs_ < b.db_costs_ ||
                    a.night_penalty_ < b.night_penalty_)) {}
    inline bool greater() const { return greater_; }
    inline bool smaller() const { return smaller_; }
    bool greater_, smaller_;
  };

  template <typename Label>
  static domination_info<Label> dominates(Label const& a, Label const& b) {
    return domination_info<Label>(a, b);
  }
};

struct late_connections_post_search_dominance {
  template <typename Label>
  struct domination_info {
    domination_info(Label const& a, Label const& b)
        : greater_(a.db_costs_ > b.db_costs_ ||
                   a.night_penalty_ > b.night_penalty_),
          smaller_(a.db_costs_ <= b.db_costs_ &&
                   a.night_penalty_ <= b.night_penalty_ &&
                   (a.db_costs_ < b.db_costs_ ||
                    a.night_penalty_ < b.night_penalty_)) {}
    inline bool greater() const { return greater_; }
    inline bool smaller() const { return smaller_; }
    bool greater_, smaller_;
  };

  template <typename Label>
  static domination_info<Label> dominates(Label const& a, Label const& b) {
    return domination_info<Label>(a, b);
  }
};

struct late_connections_filter {
  template <typename Label>
  static bool is_filtered(Label const& l) {
    return l.visited_hotel_ == late_connections::FILTERED;
  }
};

}  // namespace routing
}  // namespace motis
