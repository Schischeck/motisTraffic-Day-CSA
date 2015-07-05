#include "motis/railviz/train_retriever.h"

#include "motis/core/schedule/schedule.h"

#include "motis/railviz/edge_geo_index.h"

namespace motis {
namespace railviz {

train_retriever::train_retriever(schedule const& s) {
  edge_index_.resize(10);
  for (int clasz = 0; clasz <= 9; ++clasz) {
    edge_index_[clasz] =
        std::unique_ptr<edge_geo_index>(new edge_geo_index(clasz, s));
  }
}

train_retriever::~train_retriever() {}

std::vector<light_connection> train_retriever::connections(const time from,
                                                           const time to,
                                                           int max_count,
                                                           geo::box area) {
  std::vector<light_connection> connections;

  for (int clasz = 0; clasz <= 9; ++clasz) {
    auto edges = edge_index_[clasz]->edges(area);

    for (auto const& e : edges) {
      auto edge_conns = e->_m._route_edge._conns;
      if (edge_conns[0]._full_con->clasz != clasz) {
        continue;
      }

      for (auto const& edge_con : edge_conns) {
        if (edge_con.a_time >= from && edge_con.d_time <= to) {
          connections.push_back(edge_con);

          if (connections.size() >= max_count) {
            goto end;
          }
        }

        if (edge_con.d_time > to) {
          break;
        }
      }
    }
  }

end:
  return connections;
}

}  // namespace railviz
}  // namespace motis