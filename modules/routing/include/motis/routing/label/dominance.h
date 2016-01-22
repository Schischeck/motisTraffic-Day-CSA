#pragma once

namespace motis {
namespace routing {

template <typename... Dominators>
struct dominance;

template <typename FirstDominator, typename... RestDominators>
struct dominance<FirstDominator, RestDominators...> {
  template <typename Label>
  static bool dominates(bool could_dominate, Label const& a, Label const& b) {
    auto d = FirstDominator::dominates(a, b);
    return !d.greater() && dominance<RestDominators...>::dominates(
                               could_dominate || d.smaller(), a, b);
  }
};

template <>
struct dominance<> {
  template <typename Label>
  static bool dominates(bool, Label const&, Label const&) {
    return true;
  }
};

}  // namespace routing
}  // namespace motis