#pragma once

namespace motis {
namespace routing {

struct travel_time {
  duration travel_time_, travel_time_lb_;
};

struct travel_time_initializer {
  template <typename Label, typename LowerBounds>
  static void init(Label& l, LowerBounds& lb) {
    l.travel_time_ = l.now_ - l.start_;
    l.travel_time_lb_ = l.travel_time_ + lb.travel_time[l.node_->_id];
  }
};

struct travel_time_updater {
  template <typename Label, typename LowerBounds>
  static void update(Label& l, edge_cost const& ec, LowerBounds& lb) {
    l.travel_time_ += ec.time;
    l.travel_time_lb_ = l.travel_time_ + lb.travel_time[l.node_->_id];
  }
};

struct travel_time_dominance {
  template <typename Label>
  struct domination_info {
    domination_info(Label const& a, Label const& b)
        : greater_(a.travel_time_lb_ > b.travel_time_lb_),
          smaller_(a.travel_time_lb_ < b.travel_time_lb_) {}
    inline bool greater() const { return greater_; }
    inline bool smaller() const { return smaller_; }
    bool greater_, smaller_;
  };

  template <typename Label>
  static domination_info<Label> dominates(Label const& a, Label const& b) {
    return domination_info<Label>(a, b);
  }
};

}  // namespace routing
}  // namespace motis