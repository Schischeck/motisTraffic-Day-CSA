#include "motis/reliability/rating/connection_to_graph_data.h"

#include <exception>
#include <string>
#include <vector>

#include "motis/core/common/logging.h"
#include "motis/core/schedule/schedule.h"
#include "motis/core/journey/journey.h"
#include "motis/core/journey/journey_util.h"

#include "motis/reliability/graph_accessor.h"
#include "motis/reliability/rating/connection_rating.h"

namespace motis {
namespace reliability {
namespace rating {
namespace connection_to_graph_data {

std::vector<std::vector<connection_element>> get_elements(
    schedule const& sched, journey const& journey) {
  std::vector<std::vector<connection_element>> elements;

  // TODO(Mohammad Keyhani) it would be more efficient to find the first route
  // edge
  // and follow the route to get the succeeding elements
  foreach_light_connection(journey, [&](unsigned const dep_stop_idx,
                                        unsigned const transport_idx) {
    auto const& transport = journey.transports_.at(transport_idx);
    auto const& tail_stop = journey.stops_.at(dep_stop_idx);
    auto const& head_stop = journey.stops_.at(dep_stop_idx + 1);
    auto const element = detail::to_element(
        dep_stop_idx, sched, tail_stop.eva_no_, head_stop.eva_no_,
        unix_to_motistime(sched.schedule_begin_,
                          tail_stop.departure_.timestamp_),
        unix_to_motistime(sched.schedule_begin_, head_stop.arrival_.timestamp_),
        transport.category_id_, transport.train_nr_,
        transport.line_identifier_);

    // begin new train if elements empty or if there is an interchange
    if (elements.empty() ||
        elements.back().back().to_->id_ != element.from_->id_) {
      elements.emplace_back();
    }

    elements.back().push_back(element);
  });

  if (elements.empty()) {
    LOG(logging::error) << "Elements is empty";
    throw element_not_found_exception();
  }
  return elements;
}

connection_element get_last_element(schedule const& sched,
                                    journey const& journey) {
  for (auto it = journey.transports_.rbegin(); it != journey.transports_.rend();
       ++it) {
    auto const& transport = *it;
    if (!transport.is_walk_) {
      unsigned int const tail_stop_idx = transport.to_ - 1;
      auto const& tail_stop = journey.stops_[tail_stop_idx];
      auto const& head_stop = journey.stops_[tail_stop_idx + 1];
      return detail::to_element(
          tail_stop_idx, sched, tail_stop.eva_no_, head_stop.eva_no_,
          unix_to_motistime(sched.schedule_begin_,
                            tail_stop.departure_.timestamp_),
          unix_to_motistime(sched.schedule_begin_,
                            head_stop.arrival_.timestamp_),
          transport.category_id_, transport.train_nr_,
          transport.line_identifier_);
    }
  }
  throw element_not_found_exception();
}

namespace detail {

connection_element const to_element(
    unsigned int const departure_stop_idx, schedule const& sched,
    std::string const& tail_eva, std::string const& head_eva,
    motis::time const dep_time, motis::time const arr_time,
    unsigned int const category_id, unsigned int const train_nr,
    std::string const& line_identifier) {
  auto const& tail_station = *sched.station_nodes_.at(
      sched.eva_to_station_.find(tail_eva)->second->index_);
  auto const head_station_id =
      sched.eva_to_station_.find(head_eva)->second->index_;

  for (auto const& entering_edge : tail_station.edges_) {
    auto const route_edge =
        graph_accessor::get_departing_route_edge(*entering_edge.to_);
    if (route_edge && route_edge->to_->station_node_->id_ == head_station_id) {
      auto const light_conn = graph_accessor::find_light_connection(
          *route_edge, dep_time, true, category_id, train_nr, line_identifier);

      if (light_conn.first) {
        bool const is_first_route_node =
            graph_accessor::get_arriving_route_edge(*entering_edge.to_) ==
            nullptr;
        return connection_element(departure_stop_idx, route_edge->from_,
                                  route_edge->to_, light_conn.first,
                                  light_conn.second, is_first_route_node);
      }
    }
  }

  LOG(logging::error) << "Could not find light connection:"
                      << " tail=" << tail_eva << " head=" << head_eva
                      << " dep=" << dep_time << " arr=" << arr_time
                      << " tr=" << train_nr << " cat=" << category_id
                      << " line=" << line_identifier;

  throw element_not_found_exception();
}

}  // namespace detail
}  // namespace connection_to_graph_data
}  // namespace rating
}  // namespace reliability
}  // namespace motis
