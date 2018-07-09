#pragma once

namespace motis {
namespace routing {

constexpr unsigned MAX_TRAVEL_TIME = 1440;

struct travel_time {
  uint64_t travel_time_, travel_time_lb_;
};

struct get_travel_time_lb {
  template <typename Label>
  uint64_t operator()(Label const* l) {
    return l->travel_time_lb_;
  }
};

struct travel_time_initializer {
  template <typename Label, typename LowerBounds>
  static void init(Label& l, LowerBounds& lb) {
    l.travel_time_ = std::abs(l.now_.ts() - l.start_.ts());

    auto const lb_val = lb.travel_time_[l.get_node()];
    if (lb.travel_time_.is_reachable(lb_val)) {
      l.travel_time_lb_ = l.travel_time_ + lb_val;
    } else {
      l.travel_time_lb_ = std::numeric_limits<uint64_t>::max();
    }
  }
};

struct travel_time_updater {
  template <typename Label, typename LowerBounds>
  static void update(Label& l, edge_cost const& ec, LowerBounds& lb) {
    l.travel_time_ += ec.time_.ts();

    auto const lb_val = lb.travel_time_[l.get_node()];
    if (lb.travel_time_.is_reachable(lb_val)) {
      l.travel_time_lb_ = l.travel_time_ + lb_val;
    } else {
      l.travel_time_lb_ = std::numeric_limits<uint64_t>::max();
    }
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
          dist_(std::min(std::abs(a.start_.ts() - b.start_.ts()),
                         std::abs(a.now_.ts() - b.now_.ts()))) {}

    domination_info(time travel_time_a, time travel_time_b, time dist)
        : travel_time_a_(travel_time_a),
          travel_time_b_(travel_time_b),
          dist_(dist) {}

    inline bool greater() const {
      auto a = 0.5f *
               (static_cast<double>(travel_time_a_.ts()) / travel_time_b_.ts());
      return static_cast<double>(travel_time_a_.ts()) + a * dist_.ts() >
             travel_time_b_.ts();
    }

    inline bool smaller() const {
      auto a = 0.5f *
               (static_cast<double>(travel_time_a_.ts()) / travel_time_b_.ts());
      return static_cast<double>(travel_time_a_.ts()) + a * dist_.ts() <
             travel_time_b_.ts();
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
    return l.travel_time_lb_ > MAX_TRAVEL_TIME;
  }
};

}  // namespace routing
}  // namespace motis
