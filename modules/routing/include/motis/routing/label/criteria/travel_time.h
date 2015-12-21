#pragma once

namespace motis {
namespace routing {

struct travel_time {
  travel_time() : travel_time_(0), travel_time_lb_(0) {}
  duration travel_time_, travel_time_lb_;
};

struct travel_time_updater {
  template <typename Label, typename LowerBounds>
  void update(Label& l, edge_cost const& ec, LowerBounds& lb) {
    l.travel_time_ += ec.duration_;
    l.travel_time_lb_ = l.travel_time_ + lb.travel_time[l.node_];
  }
};

struct travel_time_dominance {
  template <bool ByTerminal, typename Label>
  struct domination_info {
    domination_info(Label const& a, Label const& b)
        : greater_(ByTerminal ? a.travel_time_lb_ > b.travel_time_lb_
                              : a.travel_time_ > b.travel_time_),
          smaller_(ByTerminal ? a.travel_time_lb_ < b.travel_time_lb_
                              : a.travel_time_ < b.travel_time_) {}
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