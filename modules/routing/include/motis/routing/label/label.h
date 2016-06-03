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

  label(edge const* e, label* pred, time now, lower_bounds& lb,
        uint8_t const slot)
      : pred_(pred),
        edge_(e),
        connection_(nullptr),
        start_(pred != nullptr ? pred->start_ : now),
        now_(now),
        dominated_(false),
        slot_(slot) {
    Init::init(*this, lb);
  }

  node const* get_node() const { return edge_->to_; }

  template <typename Edge, typename LowerBounds>
  bool create_label(label& l, Edge const& e, LowerBounds& lb) {
    if (pred_ && e.get_destination() == pred_->get_node()) {
      return false;
    }

    auto ec = e.get_edge_cost(now_, connection_);
    if (!ec.is_valid()) {
      return false;
    }

    l = *this;
    l.pred_ = this;
    l.edge_ = &e;
    l.connection_ = ec.connection_;
    l.now_ += ec.time_;
    l.slot_ = ec.slot_;

    Updater::update(l, e.type(), ec, lb);
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
  uint8_t slot_;
};

}  // namespace routing
}  // namespace motis
