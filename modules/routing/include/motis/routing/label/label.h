#pragma once

namespace motis {
namespace routing {

template <typename... DataClass>
struct label_data : public DataClass... {};

template <typename Data, typename Init, typename Updater, typename Dominance,
          typename PostSearchDominance, typename Comparator>
struct label : public Data {
  label() = default;

  label(node const* n, label* pred, time now, lower_bounds& lower_bounds)
      : pred_(pred),
        node_(n),
        connection_(nullptr),
        start_(pred != nullptr ? pred->start_ : now),
        now_(now),
        dominated_(false) {
    Init::init(*this, lower_bounds);
  }

  template <typename Edge, typename LowerBounds>
  bool create_label(label& l, Edge const& e, LowerBounds& lb) {
    auto ec = e.get_edge_cost(now_, connection_);
    if (!ec.is_valid()) {
      return false;
    }

    l = *this;
    l.pred_ = this;
    l.node_ = e.get_destination();
    l.connection_ = ec.connection;
    l.now_ += ec.time;
    Updater::update(l, ec, lb);
    return true;
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
  node const* node_;
  light_connection const* connection_;
  time start_, now_;
  bool dominated_;
};

}  // namespace routing
}  // namespace motis