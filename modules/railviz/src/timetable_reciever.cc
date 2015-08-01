#include "motis/railviz/timetable_receiver.h"

namespace motis {
namespace railviz {

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
        const station_node* next_station = next_station_on_route(n);
        const station_node* end_station_of_route =
            end_station_for_route(n._route, np);
        for (const light_connection& l : e._m._route_edge._conns) {
          if (n._route >= 0) {
            timetable_.push_back(timetable_entry(
                &l, next_station, end_station_of_route, true, n._route));
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
        const station_node* prev_station = prev_station_on_route(n);
        const station_node* start_station_of_route =
            start_station_for_route(n._route, np);
        for (const light_connection& l : e->_m._route_edge._conns) {
          if (n._route >= 0) {
            timetable_.push_back(timetable_entry(
                &l, prev_station, start_station_of_route, false, n._route));
          }
        }
      }
    }
  }
}

const motis::station_node* timetable_retriever::next_station_on_route(
    const motis::node& route_node) const {
  if (!route_node.is_route_node()) {
    std::cout << "no route node" << std::endl;
    return NULL;
  }
  unsigned int route_id = route_node._route;
  for (const edge& e : route_node._edges) {
    if (e._to->_route == route_id &&
        e._to->get_station() != route_node.get_station()) {
      return e._to->get_station();
    }
  }

  return route_node.get_station();
}

const motis::station_node* timetable_retriever::next_station_on_route(
    const motis::station_node& node, unsigned int route_id) const {
  for (const motis::node* n : node.get_route_nodes()) {
    if (n->_route == route_id) return next_station_on_route(*n);
  }
  return &node;
}

const motis::station_node* timetable_retriever::end_station_for_route(
    unsigned int route_id, const node* current_node) const {
  std::set<unsigned int> cycle_detection;
  return end_station_for_route(route_id, current_node, cycle_detection);
}

const motis::station_node* timetable_retriever::end_station_for_route(
    unsigned int route_id, const node* current_node,
    std::set<unsigned int>& cycle_detection) const {
  if (!current_node->is_route_node() && !current_node->is_station_node()) {
    std::cout << "no route or station- _node" << std::endl;
    return NULL;
  }

  const station_node* current_station;
  if (current_node->is_station_node())
    current_station = current_node->as_station_node();
  else
    current_station = current_node->get_station();

  if (cycle_detection.find(current_station->_id) != cycle_detection.end()) {
    return current_station;
  }
  cycle_detection.insert(current_station->_id);

  const station_node* next_station_node;
  if (current_node->is_station_node()) {
    next_station_node =
        next_station_on_route(*current_node->as_station_node(), route_id);
  } else {
    next_station_node = next_station_on_route(*current_node);
  }

  if (next_station_node != current_station)
    return end_station_for_route(route_id, next_station_node, cycle_detection);

  return current_station;
}

const motis::station_node* timetable_retriever::prev_station_on_route(
    const motis::node& route_node) const {
  if (!route_node.is_route_node()) {
    std::cout << "no route node" << std::endl;
    return NULL;
  }
  unsigned int route_id = route_node._route;
  for (const edge* e : route_node._incoming_edges) {
    if (e->_from->_route == route_id &&
        e->_from->get_station() != route_node.get_station()) {
      return e->_from->get_station();
    }
  }

  return route_node.get_station();
}

const motis::station_node* timetable_retriever::prev_station_on_route(
    const motis::station_node& node, unsigned int route_id) const {
  for (const motis::node* n : node.get_incoming_route_nodes()) {
    if (n->_route == route_id) return prev_station_on_route(*n);
  }
  return &node;
}

const motis::station_node* timetable_retriever::start_station_for_route(
    unsigned int route_id, const node* current_node) const {
  std::set<unsigned int> cycle_detection;
  return start_station_for_route(route_id, current_node, cycle_detection);
}

const motis::station_node* timetable_retriever::start_station_for_route(
    unsigned int route_id, const node* current_node,
    std::set<unsigned int>& cycle_detection) const {
  if (!current_node->is_route_node() && !current_node->is_station_node()) {
    std::cout << "no route or station- _node" << std::endl;
    return NULL;
  }

  const station_node* current_station;
  if (current_node->is_station_node())
    current_station = current_node->as_station_node();
  else
    current_station = current_node->get_station();

  if (cycle_detection.find(current_station->_id) != cycle_detection.end()) {
    return current_station;
  }
  cycle_detection.insert(current_station->_id);

  const station_node* prev_station_node;
  if (current_node->is_station_node()) {
    prev_station_node =
        prev_station_on_route(*current_node->as_station_node(), route_id);
  } else {
    prev_station_node = prev_station_on_route(*current_node);
  }

  if (prev_station_node != current_station)
    return start_station_for_route(route_id, prev_station_node,
                                   cycle_detection);

  return current_station;
}
}
}
