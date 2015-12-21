#pragma once

namespace motis {
namespace routing {

template <typename... Traits>
struct label_updater;

template <typename FirstUpdater, typename... RestUpdaters>
struct label_updater<FirstUpdater, RestUpdaters...> {
  template <typename Label, typename LowerBounds>
  static void update(Label& l, edge_cost const& ec, LowerBounds& lb) {
    FirstUpdater()(l, ec, lb);
    label_updater<RestUpdaters...>::update(l, ec, lb);
  }
};

template <>
struct label_updater<> {
  template <typename Label, typename LowerBounds>
  static void update(Label&, edge_cost const&, LowerBounds&) {}
};

}  // namespace routing
}  // namespace motis
