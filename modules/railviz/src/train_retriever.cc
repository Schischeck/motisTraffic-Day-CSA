#include "motis/railviz/train_retriever.h"

#include "motis/core/schedule/schedule.h"
#include "motis/railviz/edge_geo_index.h"

namespace motis {
namespace railviz {

train_retriever::train_retriever(schedule const& s) : schedule_(s) {
  edge_index_.resize(10);
  for (int clasz = 0; clasz <= 9; ++clasz) {
    edge_index_[clasz] =
        std::unique_ptr<edge_geo_index>(new edge_geo_index(clasz, s));
  }
}

train_retriever::~train_retriever() {}

std::vector<std::pair<light_connection const*, edge const*>>
train_retriever::trains(const time from, const time to, int max_count,
                        geo::box area) {
  std::vector<std::pair<light_connection const*, edge const*>> connections;
  for (int clasz = 0; clasz <= 9; ++clasz) {
    for (auto const& e : edge_index_[clasz]->edges(area)) {
      for (auto const& con : e->_m._route_edge._conns) {
        if (con.a_time >= from && con.d_time <= to) {
          connections.emplace_back(&con, e);
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
