#pragma once

namespace motis {
namespace routing {

template <typename... Dominators>
struct comparator;

template <typename FirstDominator, typename... RestDominators>
struct comparator<FirstDominator, RestDominators...> {
  template <typename Label>
  static bool lexicographical_compare(Label const& a, Label const& b) {
    auto d = FirstDominator::dominates<true>(a, b);
    if (d.smaller()) {
      return true;
    } else if (d.greater()) {
      return false;
    } else {
      return comparator<RestDominators...>::lexicographical_compare(a, b);
    }
  }
};

template <>
struct comparator<> {
  template <typename Label>
  static bool lexicographical_compare(Label const&, Label const&) {
    return false;
  }
};

}  // namespace routing
}  // namespace motis