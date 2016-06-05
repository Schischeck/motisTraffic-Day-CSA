#pragma once

#include "motis/routing/lower_bounds.h"

namespace motis {
namespace routing {

template <typename... DataClass>
struct label_data : public DataClass... {};

template <typename Data, typename Init, typename Updater, typename Filter,
          typename Dominance, typename PostSearchDominance, typename Comparator>
struct label : public Data {
  label() = default;

  label(edge const* e, label* pred, time now, lower_bounds& lb)
      : pred_(pred),
        edge_(e),
        connection_(nullptr),
        start_(pred != nullptr ? pred->start_ : now),
        now_(now),
        dominated_(false),
        used_s2s_walk_(false) {
    Init::init(*this, lb);
  }

  node const* get_node() const { return edge_->to_; }

  template <typename Edge, typename LowerBounds>
  bool create_label(label& l, Edge const& e, LowerBounds& lb,
                    bool const is_goal) {
    if ((pred_ && e.get_destination() == pred_->get_node()) ||
        (used_s2s_walk_ && is_s2s_walk(e))) {
      return false;
    }

    auto ec = e.get_edge_cost(now_, connection_);
    if (!ec.is_valid()) {
      return false;
    }
    if (is_goal) {
      correct_ec_for_terminal_label(ec, e);
    }

    l = *this;
    l.pred_ = this;
    l.edge_ = &e;
    l.connection_ = ec.connection_;
    l.now_ += ec.time_;
    l.used_s2s_walk_ =
        !removes_s2s_walk_flag(e) && (used_s2s_walk_ || is_s2s_walk(e));

    Updater::update(l, ec, lb);
    return !Filter::is_filtered(l);
  }

  bool dominates(label const& o) const {
    if (start_ < o.start_ || now_ > o.now_) {
      return false;
    }
    return Dominance::dominates(false, *this, o);
  }

  bool dominates_post_search(label const& o) const {
    return PostSearchDominance::dominates(false, *this, o);
  }

  bool operator<(label const& o) const {
    return Comparator::lexicographical_compare(*this, o);
  }

  label const* pred_;
  edge const* edge_;
  light_connection const* connection_;
  time start_, now_;
  bool dominated_;
  bool used_s2s_walk_;

private:
  template <typename Edge>
  static inline bool is_s2s_walk(Edge const& e) {
    return e.type() == edge::FOOT_EDGE && e.from_->is_foot_node() &&
           e.to_->is_station_node();
  }

  template <typename Edge>
  static inline bool removes_s2s_walk_flag(Edge const& e) {
    return e.type() == edge::ROUTE_EDGE || e.type() == edge::MUMO_EDGE ||
           e.type() == edge::PERIODIC_MUMO_EDGE || e.type() == edge::HOTEL_EDGE;
  }

  template <typename Edge>
  inline void correct_ec_for_terminal_label(edge_cost& ec, Edge const& e) {
    if (get_node()->is_route_node() && e.to_->is_station_node()) {
      ec.time_ = 0;
      ec.transfer_ = false;
    }
  }
};

}  // namespace routing
}  // namespace motis
