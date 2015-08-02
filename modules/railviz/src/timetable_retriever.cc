#include "motis/railviz/timetable_retriever.h"

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

void timetable_retriever::init( const schedule& sched )
{
  for( auto& station_node_ptr_ : sched.station_nodes ) {
    const station_node& station_node_ = *station_node_ptr_;
    for( unsigned int route_id : routes_at_station(station_node_) )
    {
      if( route_end_station.find(route_id) == route_end_station.end() )
      {
        const station_node* start_station = start_station_for_route(route_id, &station_node_);
        const station_node* end_station = end_station_for_route( route_id, &station_node_ );
        route_start_station[route_id] = start_station;
        route_end_station[route_id] = end_station;
      }
    }
  }
}

std::vector<route> timetable_retriever::get_routes_on_time(unsigned int route_id, time time) const {
  if( route_end_station.find(route_id) == route_end_station.end() )
    return {};

  std::vector<motis::time> departure_times = get_route_departure_times(route_id);
  std::vector<motis::time> arrival_times = get_route_arrival_times(route_id);
  std::vector<int> requested_tracks;

  for( int i = 0; i < departure_times.size(); ++i )
  {
    if( departure_times[i] <= time && time <= arrival_times[i] )
      requested_tracks.push_back(i);
  }

}

std::vector<std::pair<int, const light_connection*>> get_track_information( const station_node& station, unsigned int route_id, std::vector<int>& tracks ) const {
  std::vector<std::pair<int, const light_connection*>> track_information;
  for( const node* n : station.get_route_nodes() ) {
    if( n->_route == route_id ) {
      for( const edge& e : n->_edges ) {
        if( e.type() == edge::ROUTE_EDGE ) {
          for( int track : tracks ) {
            track_information.push_back(std::pair<int, const light_connection*>(track, &(e._m._route_edge._conns[track])));
          }
        }
      }
      break;
    }
  }
  return track_information;
}


void timetable_retriever::timetable_for_station_outgoing(
    const station_node& station, timetable& timetable_) const {
  for (const node* np : station.get_route_nodes()) {
    const node& n = *np;
    for (const edge& e : n._edges) {
      if (e.type() == edge::ROUTE_EDGE) {
        const station_node* next_station = next_station_on_route(n);
        const station_node* end_station_of_route =
            route_end_station.at(n._route);
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
            route_start_station.at(n._route);
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

std::vector<unsigned int> timetable_retriever::routes_at_station( const station_node& station ) const
{
  std::vector<unsigned int> route_ids;
  // skip dummie
  if( station._id == 0 )
    return route_ids;

  for( auto const* node : station.get_route_nodes() ) {
    if(node->_route > 0)
    route_ids.push_back(node->_route);
  }
  for( auto const* node : station.get_incoming_route_nodes() ) {
    if(node->_route > 0)
    route_ids.push_back(node->_route);
  }
}

std::vector<motis::time> timetable_retriever::get_route_departure_times( unsigned int route_id ) const {
  if( route_end_station.find(route_id) == route_end_station.end() )
    return {};

  std::vector<motis::time> times;
  const station_node* start_station = route_start_station.find(route_id);
  for( const auto* node : start_station->get_route_nodes() ) {
    if( node->_route == route_id ) {
      for( const edge& e : node._edges ) {
        if( e.type() == edge::ROUTE_EDGE ) {
          for( const light_connection& lc : e._m._route_edge._conns ) {
            times.push_back(lc.d_time);
          }
        }
      }
      break;
    }
  }
  return times;
}

std::vector<motis::time> timetable_retriever::get_route_arrival_times( unsigned int route_id ) const {
  if( route_end_station.find(route_id) == route_end_station.end() )
    return {};

  std::vector<motis::time> times;
  const station_node* end_station = route_end_station.find(route_id);
  for( const auto* node : end_station->get_incoming_route_nodes() ) {
    if( node->_route == route_id ) {
      for( const edge& e : node._edges ) {
        if( e.type() == edge::ROUTE_EDGE ) {
          for( const light_connection& lc : e._m._route_edge._conns ) {
            times.push_back(lc.a_time);
          }
        }
      }
      break;
    }
  }
  return times;
}

}
}
