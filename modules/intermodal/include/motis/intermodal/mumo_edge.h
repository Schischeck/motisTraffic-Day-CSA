#pragma once
#include <vector>

#include "motis/core/schedule/time.h"

#include "motis/protocol/Message_generated.h"

namespace motis {
namespace intermodal {

enum class mumo_type : int { FOOT, BIKE };

inline int to_int(mumo_type const type) {
  return static_cast<typename std::underlying_type<mumo_type>::type>(type);
}

inline std::string to_string(mumo_type const type) {
  char const* strs[] = {"foot", "bike"};
  return strs[to_int(type)];
}

struct mumo_edge {
  mumo_edge(std::string const& from, std::string const& to, duration const d,
            mumo_type const type)
      : from_(from), to_(to), duration_(d), type_(type) {}

  std::string from_, to_;
  duration duration_;
  mumo_type type_;
};

void remove_intersection(std::vector<mumo_edge>& starts,
                         std::vector<mumo_edge> const& destinations,
                         routing::SearchDir const);

std::vector<flatbuffers::Offset<routing::AdditionalEdgeWrapper>> write_edges(
    flatbuffers::FlatBufferBuilder& fbb, std::vector<mumo_edge> const& starts,
    std::vector<mumo_edge> const& destinations);

}  // namespace motis
}  // namespace motis
