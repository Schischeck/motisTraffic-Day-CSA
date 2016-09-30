#include "motis/intermodal/mumo_edge.h"

#include <algorithm>

#include "motis/core/common/erase_if.h"

#include "motis/protocol/Message_generated.h"

using namespace flatbuffers;
using namespace motis::routing;

namespace motis {
namespace intermodal {

void remove_intersection(std::vector<mumo_edge>& starts,
                         std::vector<mumo_edge> const& destinations,
                         SearchDir const dir) {
  if (dir == SearchDir_Forward) {
    for (auto const& dest : destinations) {
      erase_if(starts,
               [&dest](auto const& start) { return start.to_ == dest.from_; });
    }
  } else {
    for (auto const& dest : destinations) {
      erase_if(starts,
               [&dest](auto const& start) { return start.from_ == dest.to_; });
    }
  }
}

std::vector<Offset<AdditionalEdgeWrapper>> write_edges(
    FlatBufferBuilder& fbb, std::vector<mumo_edge> const& starts,
    std::vector<mumo_edge> const& destinations) {
  std::vector<Offset<AdditionalEdgeWrapper>> edges;

  for (auto const& edge : starts) {
    edges.push_back(CreateAdditionalEdgeWrapper(
        fbb, AdditionalEdge_MumoEdge,
        CreateMumoEdge(fbb, fbb.CreateString(edge.from_),
                       fbb.CreateString(edge.to_), edge.duration_, 0,
                       to_int(edge.type_))
            .Union()));
  }

  for (auto const& edge : destinations) {
    edges.push_back(CreateAdditionalEdgeWrapper(
        fbb, AdditionalEdge_MumoEdge,
        CreateMumoEdge(fbb, fbb.CreateString(edge.from_),
                       fbb.CreateString(edge.to_), edge.duration_, 0,
                       to_int(edge.type_))
            .Union()));
  }

  return edges;
}

}  // namespace motis
}  // namespace motis
