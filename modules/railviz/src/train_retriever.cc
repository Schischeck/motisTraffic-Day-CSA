#include "motis/railviz/train_retriever.h"

#include "motis/core/schedule/schedule.h"

#include "motis/railviz/edge_geo_index.h"

namespace motis {
namespace railviz {

train_retriever::train_retriever(schedule const& s) {
  edge_index_.resize(NUM_CLASSES);
  for (auto clasz = 0u; clasz < NUM_CLASSES; ++clasz) {
    edge_index_[clasz] = std::make_unique<edge_geo_index>(clasz, s);
  }
}

train_retriever::~train_retriever() = default;

std::vector<ev_key> train_retriever::trains(time const from, time const to,
                                            unsigned const max_count,
                                            geo::box const area) {
  std::vector<ev_key> connections;
  for (auto clasz = 0u; clasz < NUM_CLASSES; ++clasz) {
    for (auto const& e : edge_index_[clasz]->edges(area)) {
      for (auto i = 0u; i < e->m_.route_edge_.conns_.size(); ++i) {
        auto const& con = e->m_.route_edge_.conns_[i];
        if (con.a_time_ >= from && con.d_time_ <= to) {
          connections.emplace_back(ev_key{e, i, event_type::DEP});
          if (connections.size() >= max_count) {
            goto end;
          }
        }
      }
    }
  }
end:
  return connections;
}

}  // namespace railviz
}  // namespace motis
