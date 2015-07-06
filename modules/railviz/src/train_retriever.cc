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

train_list_ptr train_retriever::trains(const time from,
                                                     const time to,
                                                     geo::box area,
                                                     int max_count) {
  train_list_ptr train_list = std::unique_ptr<std::vector<train_ptr>>(new std::vector<train_ptr>);

  for (int clasz = 0; clasz <= 9; ++clasz) {
    auto edges = edge_index_[clasz]->edges(area);

    for (auto const& e : edges) {
      auto edge_conns = e->_m._route_edge._conns;
      if (edge_conns[0]._full_con->clasz != clasz) {
        continue;
      }

      for (auto const& edge_con : edge_conns) {
        if (edge_con.a_time >= from && edge_con.d_time <= to) {
          train_ptr train_(new train());
          train_.get()->d_time = edge_con.d_time;
          train_.get()->a_time = edge_con.a_time;
          train_.get()->d_station = e->_from->get_station()->_id;
          train_.get()->a_station = e->_to->get_station()->_id;
          train_list.get()->push_back(std::move(train_));
          
          if (train_list.get()->size() >= max_count) {
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
  return std::move(train_list);
}

}  // namespace railviz
}  // namespace motis
