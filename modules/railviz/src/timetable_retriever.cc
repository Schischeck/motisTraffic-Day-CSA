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

void timetable_retriever::init(const schedule& sched) {
  for (auto& station_node_ptr : sched.station_nodes) {
    const station_node& station_node_ = *station_node_ptr;
    for (unsigned int route_id : routes_at_station(station_node_)) {
      if (route_start_node.find(route_id) == route_start_node.end()) {
        const node* start_node =
            start_node_for_route(route_id, &station_node_);
        const node* end_node =
            end_node_for_route(route_id, &station_node_);

        route_start_node[route_id] = start_node;
        route_end_node[route_id] = end_node;
      }
    }
  }
}

std::vector<motis::station_node const*> timetable_retriever::stations_on_route( unsigned int route_id ) const {
  const motis::node* node_ = route_start_node.at(route_id);
  if( node_ == nullptr )
    return {};

  std::vector<motis::station_node const*> station_nodes;
  station_nodes.push_back( node_->get_station() );

  const motis::node* next_node = nullptr;
  while( (next_node = child_node(*node_)) != nullptr &&
         next_node != node_) {
    node_ = next_node;
    station_nodes.push_back( node_->get_station() );
  };

  return station_nodes;
}

std::vector<route> timetable_retriever::get_routes_on_time(
    unsigned int route_id, time time) const {
  if (route_start_node.find(route_id) == route_start_node.end()) return {};

  std::vector<motis::time> departure_times =
      get_route_departure_times(route_id);
  std::vector<motis::time> arrival_times = get_route_arrival_times(route_id);
  std::vector<int> requested_tracks;
  for (int i = 0; i < departure_times.size(); ++i) {
    if (departure_times[i] <= time && time <= arrival_times[i]) {
      requested_tracks.push_back(i);
    }
  }

  if (requested_tracks.size() == 0) {
    return {};
  }

  std::vector<route> routes;
  for (int i : requested_tracks) {
    routes.emplace_back();
  }

  const node* current_node = route_start_node.at(route_id);
  do {
    int j = 0;
    for(int i : requested_tracks) {
      routes.at(j).emplace_back(
            current_node->get_station(), current_node, get_light_connection_outgoing(*current_node, i));
      ++j;
    }
    current_node = child_node(*current_node);
  } while( current_node != nullptr );
  return routes;
}

const light_connection* timetable_retriever::get_light_connection_incoming(
    const node& node_, int index) const {
  assert( node_.is_route_node() );

  for (auto const& e : node_._incoming_edges) {
    if (e->type() == edge::ROUTE_EDGE) {
      return &(e->_m._route_edge._conns[index]);
    }
  }

  return nullptr;
}

const light_connection* timetable_retriever::get_light_connection_outgoing(
    const node& node_, int index) const {
  assert( node_.is_route_node() );

  for (const edge& e : node_._edges) {
    if (e.type() == edge::ROUTE_EDGE) {
      return &(e._m._route_edge._conns[index]);
    }
  }

  return nullptr;
}

void timetable_retriever::timetable_for_station_outgoing(
    const station_node& station, timetable& timetable_) const {
  for (const node* np : station.get_route_nodes()) {
    const node& n = *np;
    for (const edge& e : n._edges) {
      if (e.type() == edge::ROUTE_EDGE) {
        const station_node* next_station = child_node(n)->get_station();
        const station_node* end_station_of_route =
            route_end_node.at(n._route)->get_station();
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
        const station_node* prev_station = parent_node(n)->get_station();
        const station_node* start_station_of_route =
            route_start_node.at(n._route)->get_station();
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

const motis::node* timetable_retriever::parent_node(const node &node) const {
  assert(node.is_route_node());
  auto const& incoming = node._incoming_edges;
  if( incoming.size() == 0 ) {
    std::cout << "railviz: WARNING: timetable_retreiver::parent_node: no incoming edges." << std::endl;
    return nullptr;
  }
  for( auto const& edge_ : incoming ) {
    if( edge_->type() == motis::edge::ROUTE_EDGE ) {
      return edge_->_from.ptr();
    }
  }
  return nullptr;
}

const motis::node* timetable_retriever::child_node(const node &node) const {
  assert(node.is_route_node());
  auto const& outgoing = node._edges;
  if( outgoing.size() == 0 ) {
    std::cout << "railviz: WARNING: timetable_retreiver::child_node: no outgoing edges." << std::endl;
    return nullptr;
  }
  for( auto const& edge_ : outgoing ) {
    if( edge_.type() == motis::edge::ROUTE_EDGE ) {
      return edge_._to.ptr();
    }
  }
  return nullptr;
}

const motis::node* timetable_retriever::start_node_for_route(
    unsigned int route_id, const station_node* current_node ) const {
  const motis::node* node_ = current_node;
  if( current_node->get_station()->get_route_nodes().size() == 0 )
    return current_node;

  for( auto const& route_node : current_node->get_station()->get_route_nodes() ) {
    if( route_node->_route == route_id ) {
      node_ = route_node;
      break;
    }
  }

  const motis::node* parent_node_ = nullptr;
  while( (parent_node_ = parent_node(*node_)) != nullptr &&
         parent_node_ != current_node) {
    node_ = parent_node_;
  }

  return node_;
}

const motis::node* timetable_retriever::end_node_for_route(
    unsigned int route_id, const station_node* current_node) const {
  const motis::node* node_ = current_node;
  if( current_node->get_station()->get_route_nodes().size() == 0 )
    return current_node;

  for( auto const& route_node : current_node->get_station()->get_route_nodes() ) {
    if( route_node->_route == route_id ) {
      node_ = route_node;
      break;
    }
  }

  const motis::node* child_node_ = nullptr;
  while( (child_node_ = child_node(*node_)) != nullptr &&
         child_node_ != current_node) {
    node_ = child_node_;
  }

  return node_;
}

std::set<unsigned int> timetable_retriever::routes_at_station(
    const station_node& station) const {
  std::set<unsigned int> route_ids;
  // skip dummie
  if (station._id == 0) return route_ids;

  for (auto const* node : station.get_route_nodes()) {
    if (node->_route > 0) route_ids.insert(node->_route);
  }
  for (auto const* node : station.get_incoming_route_nodes()) {
    if (node->_route > 0) route_ids.insert(node->_route);
  }

  return route_ids;
}

std::vector<motis::time> timetable_retriever::get_route_departure_times(
    unsigned int route_id) const {
  if (route_start_node.find(route_id) == route_start_node.end()) return {};

  std::vector<motis::time> times;
  const motis::node* start_node = route_start_node.at(route_id);

  edge const* edge_ = nullptr;
  for( auto const& edge_i : start_node->_edges ) {
    if( edge_i.type() == motis::edge::ROUTE_EDGE ) {
      edge_ = &edge_i;
      break;
    }
  }
  if( edge_ == nullptr ) {
    std::cout << "railviz: WARNING: timetable_retreiver::get_route_departure_times: no incoming route-edges." << std::endl;
    return {};
  }

  for( auto const& lcon : edge_->_m._route_edge._conns ) {
    times.push_back(lcon.d_time);
  }

  return times;
}

std::vector<motis::time> timetable_retriever::get_route_arrival_times(
    unsigned int route_id) const {
  if (route_end_node.find(route_id) == route_end_node.end()) return {};

  std::vector<motis::time> times;
  const motis::node* end_node = route_end_node.at(route_id);
  if( end_node->_incoming_edges.size() > 1 ) {

  }
  edge const* edge_ = nullptr;
  for( auto const& edge_ptr : end_node->_incoming_edges ) {
    if( edge_ptr->type() == motis::edge::ROUTE_EDGE ) {
      edge_ = edge_ptr.ptr();
      break;
    }
  }
  if( edge_ == nullptr ) {
    std::cout << "railviz: WARNING: timetable_retreiver::get_route_departure_times: no incoming route-edges." << std::endl;
    return {};
  }

  for( auto const& lcon : edge_->_m._route_edge._conns ) {
    times.push_back(lcon.a_time);
  }

  return times;
}
}
}
