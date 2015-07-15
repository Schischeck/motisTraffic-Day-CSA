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

train_retriever::train_list_ptr train_retriever::trains(const time from,
                                                     const time to,
                                                     geo::box area,
                                                     int max_count) {

  std::vector<train> *trains = new std::vector<train>;
  train_list_ptr list(trains);

  for (int clasz = 0; clasz <= 9; ++clasz) {
    auto edges = edge_index_[clasz]->edges(area);

    for (auto const& e : edges) {
      auto edge_conns = e->_m._route_edge._conns;
      if (edge_conns[0]._full_con->clasz != clasz) {
        continue;
      }

      for (auto const& edge_con : edge_conns) {
        if (edge_con.a_time >= from && edge_con.d_time <= to) {
          train t = {
              e->_from->get_station()->_id,
              e->_to->get_station()->_id,
              edge_con.d_time,
              edge_con.a_time,
          };

          trains->push_back(t);

          if (trains->size() >= max_count) {
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
  return std::move(list);
}

}  // namespace railviz
}  // namespace motis
