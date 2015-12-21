#pragma once

namespace motis {
namespace routing {

struct transfers {
  transfers() : transfers_(0), transfers_lb_(0) {}
  uint8_t transfers_, transfers_lb_;
};

struct transfers_updater {
  template <typename Label, typename LowerBounds>
  void operator()(Label& l, edge_cost const& ec, LowerBounds& lb) {
    if (ec.interchange_) {
      ++l.transfers_;
    }
    l.transfers_lb_ = l.transfers_ + lb.transfers[l.node_];
  }
};

struct transfers_dominance {
  template <bool ByTerminal, typename Label>
  struct domination_info {
    domination_info(Label const& a, Label const& b)
        : greater_(ByTerminal ? a.transfers_lb_ > b.transfers_lb_
                              : a.transfers_ > b.transfers_),
          smaller_(ByTerminal ? a.transfers_lb_ < b.transfers_lb_
                              : a.transfers_ < b.transfers_) {}
    inline bool greater() const { return greater_; }
    inline bool smaller() const { return smaller_; }
    bool greater_, smaller_;
  };

  template <bool ByTerminal, typename Label>
  static domination_info<ByTerminal, Label> dominates(Label const& a,
                                                      Label const& b) {
    return domination_info<ByTerminal, Label>(a, b);
  }
};

}  // namespace routing
}  // namespace motis