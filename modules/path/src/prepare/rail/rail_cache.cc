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
    graph.nodes_.emplace_back(std::make_unique<rail_node>(
        node->idx(), node->id(),
        geo::latlng{node->pos()->lat(), node->pos()->lng()}));
  }

  for (auto const& node : *GetRailGraph(buf.buf_)->nodes()) {
    for (auto const& link : *node->links()) {
      auto& from = graph.nodes_[node->idx()];
      auto const& to = graph.nodes_[link->to()];

      from->links_.emplace_back(
          link->id(), utl::to_vec(*link->polyline(),
                                  [](auto const& pos) {
                                    return geo::latlng{pos->lat(), pos->lng()};
                                  }),
          link->dist(), from.get(), to.get());
    }
  }

  return graph;
}

void store_rail_graph(std::string const& filename, rail_graph const& graph) {
  FlatBufferBuilder fbb;
  auto const fbs_nodes = utl::to_vec(graph.nodes_, [&](auto const& node) {
    Position pos{node->pos_.lat_, node->pos_.lng_};
    return CreateRailNode(
        fbb, node->idx_, node->id_, &pos,
        fbb.CreateVector(utl::to_vec(node->links_, [&](auto const& link) {
          auto const positions = utl::to_vec(
              link.polyline_,
              [](auto const& pos) { return Position(pos.lat_, pos.lng_); });

          return CreateRailLink(fbb, link.to_->idx_, link.id_,
                                fbb.CreateVectorOfStructs(positions),
                                link.dist_);
        })));
  });

  fbb.Finish(CreateRailGraph(fbb, fbb.CreateVector(fbs_nodes)));
  file{filename.c_str(), "w+"}.write(fbb.GetBufferPointer(), fbb.GetSize());
}

}  // namespace path
}  // namespace motis
