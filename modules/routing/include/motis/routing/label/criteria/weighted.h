#pragma once

namespace motis {
namespace routing {

constexpr auto TRANSFER_COST = 20;

struct weighted {
  duration weighted_, weighted_lb_;
};

struct weighted_initializer {
  template <typename Label, typename LowerBounds>
  static void init(Label& l, LowerBounds& lb) {
    l.weighted_ = l.now_ - l.start_;
    l.weighted_lb_ = l.weighted_ + lb.travel_time_[l.get_node()->id_] +
                     lb.transfers_[l.get_node()->id_] * TRANSFER_COST;
  }
};

struct weighted_updater {
  template <typename Label, typename LowerBounds>
  static void update(Label& l, edge_cost const& ec, LowerBounds& lb) {
    l.weighted_ += ec.time_;
    if (ec.transfer_) {
      l.weighted_ += TRANSFER_COST;
    }

    l.weighted_lb_ = l.weighted_ + lb.travel_time_[l.get_node()->id_] +
                     lb.transfers_[l.get_node()->id_] * TRANSFER_COST;
  }
};

struct weighted_dominance {
  template <typename Label>
  struct domination_info {
    domination_info(Label const& a, Label const& b)
        : greater_(a.weighted_lb_ > b.weighted_lb_),
          smaller_(a.weighted_lb_ < b.weighted_lb_) {}
    inline bool greater() const { return greater_; }
    inline bool smaller() const { return smaller_; }
    bool greater_, smaller_;
  };

  template <typename Label>
  static domination_info<Label> dominates(Label const& a, Label const& b) {
    return domination_info<Label>(a, b);
  }
};

struct weighted_filter {
  template <typename Label>
  static bool is_filtered(Label const& l) {
    return l.weighted_lb_ > (1440 + TRANSFER_COST * 6);
  }
};

}  // namespace routing
}  // namespace motis
