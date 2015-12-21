#pragma once

namespace motis {
namespace routing {

template <typename... DataClass>
struct label_data : public DataClass... {};

template <typename Data, typename Updater, typename Dominator>
struct label : public Data {
  template <typename Edge, typename LowerBounds>
  bool create_label(label& l, Edge const& e, LowerBounds& lb) {
    auto ec = e.get_edge_cost(now_);
    if (!ec.is_valid()) {
      return false;
    }

    l = *this;
    LabelUpdater::update(l, ec, lb);
    return l;
  }

  bool dominates(label const& o) {
    if (_start < o._start || _now > o._now) {
      return false;
    }
    return LabelDominator::dominates<false>(false, *this, o);
  }

  bool dominates_hard(label const& o) {
    return LabelDominator::dominates<true>(false, *this, o);
  }

  label* pred_;
  node* node_;
  light_connection const* _connection;
  time start_, now_;
  bool dominated_;
};

}  // namespace routing
}  // namespace motis