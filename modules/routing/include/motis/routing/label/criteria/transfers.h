#pragma once

#include "motis/core/schedule/edges.h"

namespace motis {
namespace routing {

struct transfers {
  uint8_t transfers_, transfers_lb_;
};

struct transfers_initializer {
  template <typename Label, typename LowerBounds>
  static void init(Label& l, LowerBounds& lb) {
    l.transfers_ = 0;
    l.transfers_lb_ = lb.transfers_[l.node_->id_];
  }
};

struct transfers_updater {
  template <typename Label, typename LowerBounds>
  static void update(Label& l, edge_cost const& ec, LowerBounds& lb) {
    if (ec.transfer_) {
      ++l.transfers_;
    }
    l.transfers_lb_ = l.transfers_ + lb.transfers_[l.node_->id_];
  }
};

struct transfers_dominance {
  template <typename Label>
  struct domination_info {
    domination_info(Label const& a, Label const& b)
        : greater_(a.transfers_lb_ > b.transfers_lb_),
          smaller_(a.transfers_lb_ < b.transfers_lb_) {}
    inline bool greater() const { return greater_; }
    inline bool smaller() const { return smaller_; }
    bool greater_, smaller_;
  };

  template <typename Label>
  static domination_info<Label> dominates(Label const& a, Label const& b) {
    return domination_info<Label>(a, b);
  }
};

struct transfers_filter {
  template <typename Label>
  static bool is_filtered(Label const& l) {
    return l.transfers_lb_ > 6;
  }
};

}  // namespace routing
}  // namespace motis