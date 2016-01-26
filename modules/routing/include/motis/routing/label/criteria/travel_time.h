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

struct travel_time_alpha_dominance {
  template <typename Label>
  struct domination_info {
    domination_info(Label const& a, Label const& b)
        : travel_time_a_(a.travel_time_),
          travel_time_b_(b.travel_time_),
          dist_(std::min(std::abs(a.start_ - b.start_),
                         std::abs(a.now_ - b.now_))) {}

    inline bool greater() const {
      auto a = 0.5f * (static_cast<double>(travel_time_b_) / travel_time_a_);
      return travel_time_b_ + a * dist_ < travel_time_a_;
    }

    inline bool smaller() const {
      auto a = 0.5f * (static_cast<double>(travel_time_a_) / travel_time_b_);
      return travel_time_a_ + a * dist_ < travel_time_b_;
    }

    time travel_time_a_, travel_time_b_;
    time dist_;
  };

  template <typename Label>
  static domination_info<Label> dominates(Label const& a, Label const& b) {
    return domination_info<Label>(a, b);
  }
};

struct travel_time_filter {
  template <typename Label>
  static bool is_filtered(Label const& l) {
    return l.travel_time_lb_ > 1440;
  }
};

struct waiting_time_filter {
  template <typename Label>
  static bool is_filtered(Label const& l) {
    auto const& c = l.connection_;
    unsigned con_time = c != nullptr ? c->a_time - c->d_time : 0;
    return l.travel_time_ - l.pred_->travel_time_ - con_time > 200;
  }
};

}  // namespace routing
}  // namespace motis