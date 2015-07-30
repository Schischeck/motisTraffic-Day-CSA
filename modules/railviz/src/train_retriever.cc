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

std::vector<train> train_retriever::trains(const time from, const time to,
                                           geo::box area, int max_count) {

  std::vector<train> trains;

  for (int clasz = 0; clasz <= 9; ++clasz) {
    auto edges = edge_index_[clasz]->edges(area);

    for (auto& e : edges) {
      if (e->type() != edge::ROUTE_EDGE) continue;

      auto& edge_conns = e->_m._route_edge._conns;
      if ((int)edge_conns[0]._full_con->clasz != clasz) {
        continue;
      }

      for (auto const& edge_con : edge_conns) {
        // std::cout << edge_con.d_time << "-" << edge_con.a_time << std::endl;
        if (edge_con.a_time >= from && edge_con.d_time <= to) {
          unsigned int route_id;
          if (e->_from->is_route_node())
            route_id = e->_from->_route;
          else
            route_id = e->_to->_route;

          trains.emplace_back(train{e->_from->get_station()->_id,
                                    e->_to->get_station()->_id, route_id,
                                    &edge_con});

          if (trains.size() >= max_count) {
            goto end;
          }
        }
      }
    }
  }
end:
  return trains;
}

std::vector<train> train_retriever::timetable_for_station_outgoing(
    const station_node& station) {
  std::vector<train> trains;
  for (const node* np : station.get_route_nodes()) {
    const node& n = *np;
    for (const edge& e : n._edges) {
      if (e.type() == edge::ROUTE_EDGE) {
        std::cout << "ROUTE_EDGE" << std::endl;
        const station_node* end_station = end_station_for_route(n._route, np);
        for (const light_connection& l : e._m._route_edge._conns) {
          if (n._route >= 0) {
            train t;
            t.d_station = station._id;
            t.a_station = end_station->_id;
            t.route_id = n._route;
            t.light_conenction_ = &l;
            trains.push_back(t);
          }
        }
      }
    }
  }
  return trains;
}

geo::box train_retriever::bounds() {
  geo::coord p_max, p_min;
  p_max.lat = p_max.lng = std::numeric_limits<double>::min();
  p_min.lat = p_min.lng = std::numeric_limits<double>::max();
  for (auto& geo_index_p : edge_index_) {
    edge_geo_index& geo_index = *geo_index_p.get();
    geo::box bounds = geo_index.get_bounds();
    geo::coord max_coord = bounds.max();
    geo::coord min_coord = bounds.min();
    if (max_coord.lat > p_max.lat) {
      p_max.lat = max_coord.lat;
    }
    if (max_coord.lng > p_max.lng) {
      p_max.lng = max_coord.lng;
    }
    if (min_coord.lat < p_min.lat) {
      p_min.lat = min_coord.lat;
    }
    if (min_coord.lat < p_min.lat) {
      p_min.lat = min_coord.lat;
    }
  }
  geo::box bounds;
  bounds.first = p_min;
  bounds.second = p_max;
  return bounds;
}

const motis::station_node* train_retriever::end_station_for_route(
    unsigned int route_id) const {
  node* first_node = schedule_.route_index_to_first_route_node.at(route_id);
  return end_station_for_route(route_id, first_node);
}

const motis::station_node* train_retriever::end_station_for_route(
    unsigned int route_id, const node* current_node) const {
  if (current_node->is_foot_node()) {
    std::cout << "foot_node in end_station_for_route" << std::endl;
    return NULL;
  }

  const station_node* next_station;
  if (!current_node->is_station_node()) {
    next_station = current_node->get_station();
    return end_station_for_route(route_id, next_station);
  }

  next_station = current_node->as_station_node();
  bool next_station_found = false;
  for (const node* n : next_station->get_route_nodes()) {
    if (n->_route == route_id && n->_edges.size() > 0) {
      if (n->_edges[0].get_destination()->get_station() == next_station)
        return next_station;
      next_station = n->_edges[0].get_destination()->get_station();
      next_station_found = true;
      break;
    }
  }

  if (next_station_found) {
    return end_station_for_route(route_id, next_station);
  }

  return next_station;
}

}  // namespace railviz
}  // namespace motis
