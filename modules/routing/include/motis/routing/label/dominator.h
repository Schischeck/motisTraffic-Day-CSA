#pragma once

namespace motis {
namespace routing {

template <typename... Dominators>
struct dominance;

template <typename FirstDominator, typename... RestDominators>
struct dominance<FirstDominator, RestDominators...> {
  template <bool ByTerminal, typename Label>
  static bool dominates(bool could_dominate, Label const& a, Label const& b) {
    auto d = FirstDominator::dominates<ByTerminal>(a, b);
    return !d.greater() && dominance<RestDominators...>::dominates<ByTerminal>(
                               could_dominate || d.smaller(), a, b);
  }
};

template <>
struct dominance<> {
  template <bool ByTerminal, typename Label>
  static bool dominates(bool could_dominate, Label const& a, Label const& b) {
    return true;
  }
};

}  // namespace routing
}  // namespace motis