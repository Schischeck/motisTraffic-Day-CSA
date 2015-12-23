#pragma once

namespace motis {
namespace routing {

template <typename... Traits>
struct updater;

template <typename FirstUpdater, typename... RestUpdaters>
struct updater<FirstUpdater, RestUpdaters...> {
  template <typename Label, typename LowerBounds>
  static void update(Label& l, edge_cost const& ec, LowerBounds& lb) {
    FirstUpdater()(l, ec, lb);
    updater<RestUpdaters...>::update(l, ec, lb);
  }
};

template <>
struct updater<> {
  template <typename Label, typename LowerBounds>
  static void update(Label&, edge_cost const&, LowerBounds&) {}
};

}  // namespace routing
}  // namespace motis
