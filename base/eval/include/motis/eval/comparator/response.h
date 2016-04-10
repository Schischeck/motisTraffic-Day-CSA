#pragma once

#include <tuple>

#include "motis/protocol/RoutingResponse_generated.h"

namespace motis {
namespace eval {
namespace comparator {

typedef std::tuple<unsigned, unsigned, unsigned> connection;
enum connectionAccessors { DURATION, TRANSFERS, PRICE };

template <std::size_t Index, typename... Tp>
inline typename std::enable_if<Index == sizeof...(Tp), bool>::type dominates(
    std::tuple<Tp...> const&, std::tuple<Tp...> const&, bool could_dominate) {
  return could_dominate;
}

template <std::size_t Idx, typename... Criteria>
    inline typename std::enable_if <
    Idx<sizeof...(Criteria), bool>::type dominates(
        std::tuple<Criteria...> const& c1, std::tuple<Criteria...> const& c2,
        bool could_dominate) {
  if (std::get<Idx>(c1) > std::get<Idx>(c2)) {
    return false;
  } else if (could_dominate || std::get<Idx>(c1) < std::get<Idx>(c2)) {
    return dominates<Idx + 1, Criteria...>(c1, c2, true);
  } else {
    return dominates<Idx + 1, Criteria...>(c1, c2, false);
  }
}

template <typename... Criteria>
inline bool dominates(std::tuple<Criteria...> const& c1,
                      std::tuple<Criteria...> const& c2) {
  return dominates<0, Criteria...>(c1, c2, false);
}

struct response {
  response(routing::RoutingResponse const* r);
  std::vector<connection> connections;
};

}  // namespace comparator
}  // namespace eval
}  // namespace motis