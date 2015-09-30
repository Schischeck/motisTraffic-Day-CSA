#include "motis/railviz/timetable_retriever.h"

namespace motis {
namespace railviz {

std::vector<motis::station_node const*> timetable_retriever::stations_on_route(
    const motis::node& node) const {
  assert(node.is_route_node());
  const motis::node* node_ = start_node_for_route(node);
  if (node_ == nullptr) return {};

  std::vector<motis::station_node const*> station_nodes;
  station_nodes.push_back(node_->get_station());

  const motis::node* next_node = nullptr;
  while ((next_node = child_node(*node_)) != nullptr && next_node != node_) {
    node_ = next_node;
    station_nodes.push_back(node_->get_station());
  };

  return station_nodes;
}

std::vector<route> timetable_retriever::get_routes_on_time(
    const motis::node& node, time time) const {
  assert(node.is_route_node());

  std::vector<motis::time> departure_times = get_route_departure_times(node);

  std::vector<motis::time> requested_departure_times;
  for (int i = 0; i < departure_times.size(); ++i) {
    if (departure_times[i] <= time) {
      requested_departure_times.push_back(departure_times[i]);
    }
  }

  if (requested_departure_times.size() == 0) {
    return {};
  }

  std::vector<route> routes;

  const motis::node* start_node = start_node_for_route(node);
  for (motis::time departure_time : requested_departure_times) {
    route r;
    motis::time current_time = departure_time;
    const motis::node* current_node = start_node;
    while (current_node != nullptr) {
      const motis::light_connection* lcon = nullptr;
      for (auto const& edge : current_node->_edges) {
        if (edge.type() == motis::edge::ROUTE_EDGE) {
          lcon = edge.get_connection(current_time);
        }
      }

      r.emplace_back(current_node->get_station(), current_node, lcon);
      if (lcon != nullptr) {
        current_time = lcon->a_time;
      }
      current_node = child_node(*current_node);
    }
    if (r.size() >= 2 && std::get<2>(r.at(r.size() - 2)) != nullptr) {
      if (std::get<2>(r.at(r.size() - 2))->a_time >= time) {
        routes.push_back(r);
      }
    }
  }

  return routes;
}

timetable timetable_retriever::ordered_timetable_for_station(
    const station_node& station) const {
  timetable timetable_;
  timetable_for_station_incoming(station, timetable_);
  timetable_for_station_outgoing(station, timetable_);
  std::sort(timetable_.begin(), timetable_.end(), timetable_sort);
  return timetable_;
}

void timetable_retriever::timetable_for_station_outgoing(
    const station_node& station, timetable& timetable_) const {
  for (const node* np : station.get_route_nodes()) {
    const node& n = *np;
    for (const edge& e : n._edges) {
      if (e.type() == edge::ROUTE_EDGE) {
        const station_node* current_station = n.get_station();
        const station_node* next_station = child_node(n)->get_station();
        const station_node* end_station_of_route =
            end_node_for_route(n)->get_station();
        for (const light_connection& l : e._m._route_edge._conns) {
          if (n._route >= 0) {
            timetable_.push_back(
                timetable_entry(&l, current_station, next_station,
                                end_station_of_route, true, n._route));
          }
        }
      }
    }
  }
}

void timetable_retriever::timetable_for_station_incoming(
    const station_node& station, timetable& timetable_) const {
  for (const node* np : station.get_route_nodes()) {
    const node& n = *np;
    for (const edge* e : n._incoming_edges) {
      if (e->type() == edge::ROUTE_EDGE) {
        const station_node* current_station = n.get_station();
        const station_node* prev_station = parent_node(n)->get_station();
        const station_node* start_station_of_route =
            start_node_for_route(n)->get_station();
        for (const light_connection& l : e->_m._route_edge._conns) {
          if (n._route >= 0) {
            timetable_.push_back(
                timetable_entry(&l, current_station, prev_station,
                                start_station_of_route, false, n._route));
          }
        }
      }
    }
  }
}

const motis::node* timetable_retriever::parent_node(const node& node) const {
  assert(node.is_route_node());
  auto const& incoming = node._incoming_edges;
  if (incoming.size() == 0) {
    return nullptr;
  }
  for (auto const& edge_ : incoming) {
    if (edge_->type() == motis::edge::ROUTE_EDGE) {
      return edge_->_from.ptr();
    }
  }
  return nullptr;
}

const motis::node* timetable_retriever::child_node(const node& node) const {
  assert(node.is_route_node());
  auto const& outgoing = node._edges;
  if (outgoing.size() == 0) {
    return nullptr;
  }
  for (auto const& edge_ : outgoing) {
    if (edge_.type() == motis::edge::ROUTE_EDGE) {
      return edge_._to.ptr();
    }
  }
  return nullptr;
}

const motis::node* timetable_retriever::start_node_for_route(
    const motis::node& node_) const {
  assert(node_.is_route_node());

  const motis::node* parent_node_ = nullptr;
  const motis::node* start_node = &node_;
  while ((parent_node_ = parent_node(*start_node)) != nullptr &&
         parent_node_ != start_node) {
    start_node = parent_node_;
  }

  if (start_node == nullptr) return &node_;
  return start_node;
}

const motis::node* timetable_retriever::end_node_for_route(
    const motis::node& node_) const {
  assert(node_.is_route_node());

  const motis::node* child_node_ = nullptr;
  const motis::node* end_node = &node_;
  while ((child_node_ = child_node(*end_node)) != nullptr &&
         child_node_ != end_node) {
    end_node = child_node_;
  }

  if (end_node == nullptr) return &node_;
  return end_node;
}

std::vector<motis::time> timetable_retriever::get_route_departure_times(
    const motis::node& node_) const {
  assert(node_.is_route_node());

  std::vector<motis::time> times;
  const motis::node* start_node = start_node_for_route(node_);

  edge const* edge_ = nullptr;
  for (auto const& edge_i : start_node->_edges) {
    if (edge_i.type() == motis::edge::ROUTE_EDGE) {
      edge_ = &edge_i;
      break;
    }
  }
  if (edge_ == nullptr) {
    return {};
  }

  for (auto const& lcon : edge_->_m._route_edge._conns) {
    times.push_back(lcon.d_time);
  }

  return times;
}
}
}
