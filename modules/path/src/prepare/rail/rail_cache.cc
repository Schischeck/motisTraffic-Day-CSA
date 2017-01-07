#include "motis/path/prepare/rail/rail_cache.h"

#include <memory>

#include "parser/file.h"

#include "utl/to_vec.h"

#include "motis/path/fbs/RailGraph_generated.h"

using namespace flatbuffers;
using namespace parser;

namespace motis {
namespace path {

rail_graph load_rail_graph(std::string const& filename) {
  auto const buf = file{filename.c_str(), "r"}.content();

  rail_graph graph;
  for (auto const& node : *GetRailGraph(buf.buf_)->nodes()) {
    graph.nodes_.emplace_back(
        std::make_unique<rail_node>(node->idx(), node->id()));
  }

  for (auto const& edge : *GetRailGraph(buf.buf_)->edges()) {
    auto* from = graph.nodes_[edge->from()].get();
    auto* to = graph.nodes_[edge->to()].get();

    auto const info_idx = graph.infos_.size();

    graph.infos_.emplace_back(std::make_unique<rail_edge_info>(
        utl::to_vec(*edge->polyline(),
                    [](auto const& pos) {
                      return geo::latlng{pos->lat(), pos->lng()};
                    }),
        edge->dist(), from, to));

    from->edges_.emplace_back(info_idx, true, edge->dist(), from, to);
    to->edges_.emplace_back(info_idx, false, edge->dist(), to, from);
  }
  return graph;
}

void store_rail_graph(std::string const& filename, rail_graph const& graph) {
  FlatBufferBuilder fbb;
  auto const fbs_nodes = utl::to_vec(graph.nodes_, [&](auto const& node) {
    return CreateRailNode(fbb, node->idx_, node->id_);
  });

  auto const fbs_edges = utl::to_vec(graph.infos_, [&](auto const& info) {
    auto const positions = utl::to_vec(info->polyline_, [](auto const& pos) {
      return Position(pos.lat_, pos.lng_);
    });
    return CreateRailEdge(fbb, info->nodes_.first->idx_,
                          info->nodes_.second->idx_,
                          fbb.CreateVectorOfStructs(positions), info->dist_);
  });

  fbb.Finish(CreateRailGraph(fbb, fbb.CreateVector(fbs_nodes),
                             fbb.CreateVector(fbs_edges)));
  file{filename.c_str(), "w+"}.write(fbb.GetBufferPointer(), fbb.GetSize());
}

}  // namespace path
}  // namespace motis
