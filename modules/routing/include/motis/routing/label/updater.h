#pragma once

namespace motis {
namespace routing {

template <typename... Traits>
struct updater;

template <typename FirstUpdater, typename... RestUpdaters>
struct updater<FirstUpdater, RestUpdaters...> {
  template <typename Label, typename LowerBounds>
  static void update(Label& l, uint8_t t, edge_cost const& ec,
                     LowerBounds& lb) {
    FirstUpdater::update(l, t, ec, lb);
    updater<RestUpdaters...>::update(l, t, ec, lb);
  }
};

template <>
struct updater<> {
  template <typename Label, typename LowerBounds>
  static void update(Label&, uint8_t, edge_cost const&, LowerBounds&) {}
};

}  // namespace routing
}  // namespace motis
