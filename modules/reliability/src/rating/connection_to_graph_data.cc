#include "motis/reliability/rating/connection_to_graph_data.h"

#include <exception>
#include <string>
#include <vector>

#include "motis/core/journey/journey.h"
#include "motis/core/journey/journey_util.h"
#include "motis/core/schedule/schedule.h"

#include "motis/reliability/graph_accessor.h"
#include "motis/reliability/rating/connection_rating.h"

namespace motis {
namespace reliability {
namespace rating {
namespace connection_to_graph_data {

std::vector<std::vector<connection_element>> get_elements(
    schedule const& sched, journey const& journey) {
  std::vector<std::vector<connection_element>> elements;

  // TODO it would be more efficient to find the first route edge
  // and follow the route to get the succeeding elements
  foreach_light_connection(journey, [&](journey::transport const& transport,
                                        journey::stop const& tail_stop,
                                        journey::stop const& head_stop) {
    auto const element = detail::to_element(
        tail_stop.index, sched, tail_stop.eva_no, head_stop.eva_no,
        unix_to_motistime(sched.schedule_begin_, tail_stop.departure.timestamp),
        unix_to_motistime(sched.schedule_begin_, head_stop.arrival.timestamp),
        transport.route_id, transport.category_id, transport.train_nr,
        transport.line_identifier);
    if (element.empty()) {
      throw element_not_found_exception();
    }

    // begin new train if elements empty or if there is an interchange
    if (elements.size() == 0 ||
        elements.back().back().to_->_id != element.from_->_id) {
      elements.emplace_back();
    }

    elements.back().push_back(element);
  });

  if (elements.empty()) {
    throw element_not_found_exception();
  }
  return elements;
}

connection_element get_last_element(schedule const& sched,
                                    journey const& journey) {
  for (auto it = journey.transports.rbegin(); it != journey.transports.rend();
       ++it) {
    auto const& transport = *it;
    if (transport.type == journey::transport::PublicTransport) {
      unsigned int const tail_stop_idx = transport.to - 1;
      auto const& tail_stop = journey.stops[tail_stop_idx];
      auto const& head_stop = journey.stops[tail_stop_idx + 1];
      return detail::to_element(
          tail_stop_idx, sched, tail_stop.eva_no, head_stop.eva_no,
          unix_to_motistime(sched.schedule_begin_,
                            tail_stop.departure.timestamp),
          unix_to_motistime(sched.schedule_begin_, head_stop.arrival.timestamp),
          transport.route_id, transport.category_id, transport.train_nr,
          transport.line_identifier);
    }
  }
  throw element_not_found_exception();
}

namespace detail {

connection_element const to_element(
    unsigned int const departure_stop_idx, schedule const& sched,
    std::string const& tail_eva, std::string const& head_eva,
    motis::time const dep_time, motis::time const arr_time,
    unsigned int const route_id, unsigned int const category_id,
    unsigned int const train_nr, std::string const& line_identifier) {
  auto const& tail_station = *sched.station_nodes.at(
      sched.eva_to_station.find(tail_eva)->second->index);
  auto const head_station_id =
      (unsigned int)sched.eva_to_station.find(head_eva)->second->index;

  for (auto const& entering_edge : tail_station._edges) {
    /* node: there could be multiple route nodes with the same route id
     * at the same station */
    if (static_cast<unsigned int>(entering_edge._to->_route) == route_id) {
      auto const route_edge =
          graph_accessor::get_departing_route_edge(*entering_edge._to);
      if (route_edge &&
          route_edge->_to->_station_node->_id == head_station_id) {
        auto const light_conn = graph_accessor::find_light_connection(
            *route_edge, dep_time, category_id, train_nr, line_identifier);

        if (light_conn.first) {
          bool const is_first_route_node =
              graph_accessor::get_arriving_route_edge(*entering_edge._to) ==
              nullptr;
          return connection_element(departure_stop_idx, route_edge->_from,
                                    route_edge->_to, light_conn.first,
                                    light_conn.second, is_first_route_node);
        }
      }
    }
  }

  std::cout << "\nWarning(connection_to_graph_data): Could not find light "
            << "connection of train " << train_nr << " with times " << dep_time
            << " - " << arr_time << " and stations " << tail_eva << " - "
            << head_eva << " and route_id " << route_id << std::endl;

  throw element_not_found_exception();
}

}  // namespace detail
}  // namespace connection_to_graph_data
}  // namespace rating
}  // namespace reliability
}  // namespace motis
