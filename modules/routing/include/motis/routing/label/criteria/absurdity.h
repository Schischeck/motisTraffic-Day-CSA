#pragma once

#include "motis/core/schedule/edges.h"

namespace motis {
namespace routing {

struct absurdity {
  uint8_t absurdity_, foot_counter_;
};

struct absurdity_initializer {
  template <typename Label, typename LowerBounds>
  static void init(Label& l, LowerBounds& lb) {
    l.absurdity_ = 0;
    l.foot_counter_ = 0;
  }
};

struct absurdity_updater {
  template <typename Label, typename LowerBounds>
  static void update(Label& l, edge_cost const& ec, LowerBounds& lb) {
    auto constexpr MAX_SUCCESSIVE_FOOT_EDGES_ALLOWED = 3;
    if (l.edge_->type() == edge::FOOT_EDGE ||
        l.edge_->type() == edge::AFTER_TRAIN_FOOT_EDGE) {
      ++l.foot_counter_;
      if (l.foot_counter_ > MAX_SUCCESSIVE_FOOT_EDGES_ALLOWED &&
          l.absurdity_ < UINT8_MAX) {
        ++l.absurdity_;
      }
    } else {
      l.foot_counter_ = 0;
    }
  }
};

struct absurdity_dominance {
  template <typename Label>
  struct domination_info {
    domination_info(Label const&, Label const&) {}
    inline bool greater() const { return false; }
    inline bool smaller() const { return false; }
  };

  template <typename Label>
  static domination_info<Label> dominates(Label const& a, Label const& b) {
    return domination_info<Label>(a, b);
  }
};

struct absurdity_filter {
  template <typename Label>
  static bool is_filtered(Label const& l) {
    return false;
  }
};

}  // namespace routing
}  // namespace motis
