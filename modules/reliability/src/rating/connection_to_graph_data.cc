#include "motis/reliability/rating/connection_to_graph_data.h"

#include <string>
#include <vector>

#include "motis/core/common/journey.h"
#include "motis/core/schedule/schedule.h"

#include "motis/reliability/graph_accessor.h"
#include "motis/reliability/rating/connection_rating.h"

namespace motis {
namespace reliability {
namespace rating {
namespace connection_to_graph_data {

std::pair<bool, std::vector<std::vector<connection_element>>> get_elements(
    schedule const& sched, journey const& journey) {
  std::vector<std::vector<connection_element>> elements;
  for (auto const& transport : journey.transports) {
    if (!transport.walk) {
      /* todo: it would be more efficient to find the first route edge
       * and follow the route to get the succeeding elements */
      for (auto stop_idx = transport.from; stop_idx < transport.to;
           ++stop_idx) {
        auto const& tail_stop = journey.stops[stop_idx];
        auto const& head_stop = journey.stops[stop_idx + 1];
        auto const element = detail::to_element(
            stop_idx, sched, tail_stop.eva_no, head_stop.eva_no,
            unix_to_motistime(sched.schedule_begin_,
                              tail_stop.departure.timestamp),
            unix_to_motistime(sched.schedule_begin_,
                              head_stop.arrival.timestamp),
            transport.category_id, transport.train_nr,
            transport.line_identifier);
        if (element.empty()) {
          return std::make_pair(false, elements);
        }

        // begin new train if elements empty or if there is an interchange
        if (elements.size() == 0 ||
            elements.back().back().to_->_id != element.from_->_id) {
          elements.emplace_back();
        }

        elements.back().push_back(element);
      }  // for stops
    }  // if !walk
  }  // for transports
  return std::make_pair(true, elements);
}

connection_element get_last_element(schedule const& sched,
                                    journey const& journey) {
  for (auto it = journey.transports.rbegin(); it != journey.transports.rend();
       ++it) {
    auto const& transport = *it;
    if (!transport.walk) {
      unsigned int const tail_stop_idx = transport.to - 1;
      auto const& tail_stop = journey.stops[tail_stop_idx];
      auto const& head_stop = journey.stops[tail_stop_idx + 1];
      return detail::to_element(
          tail_stop_idx, sched, tail_stop.eva_no, head_stop.eva_no,
          unix_to_motistime(sched.schedule_begin_,
                            tail_stop.departure.timestamp),
          unix_to_motistime(sched.schedule_begin_, head_stop.arrival.timestamp),
          transport.category_id, transport.train_nr, transport.line_identifier);
    }
  }
  assert(false);
  return connection_element();
}

namespace detail {

connection_element const to_element(
    unsigned int const departure_stop_idx, schedule const& sched,
    std::string const& tail_eva, std::string const& head_eva,
    motis::time const dep_time, motis::time const arr_time,
    unsigned int const category_id, unsigned int const train_nr,
    std::string const& line_identifier) {
  auto const& tail_station = *sched.station_nodes.at(
      sched.eva_to_station.find(tail_eva)->second->index);
  auto const head_station_id =
      sched.eva_to_station.find(head_eva)->second->index;

  for (auto it = tail_station._edges.begin(); it != tail_station._edges.end();
       ++it) {
    auto const route_edge = graph_accessor::get_departing_route_edge(*it->_to);
    if (route_edge && route_edge->_to->_station_node->_id == head_station_id) {
      auto const light_conn = graph_accessor::find_light_connection(
          *route_edge, dep_time, category_id, train_nr, line_identifier);

      if (light_conn.first) {
        bool const is_first_route_node =
            graph_accessor::get_arriving_route_edge(*it->_to) == nullptr;
        return connection_element(departure_stop_idx, route_edge->_from,
                                  route_edge->_to, light_conn.first,
                                  light_conn.second, is_first_route_node);
      }
    }
  }
  std::cout << "\nWarning(connection_to_graph_data): Could not find light "
            << "connection of train " << train_nr << " with times " << dep_time
            << " - " << arr_time << " and stations " << tail_eva << " - "
            << head_eva << std::endl;
  assert(false);
  // empty element (unexpected case)
  return connection_element();
}

}  // namespace detail
}  // namespace connection_to_graph_data
}  // namespace rating
}  // namespace reliability
}  // namespace motis
