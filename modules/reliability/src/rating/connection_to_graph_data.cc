#include "motis/reliability/rating/connection_to_graph_data.h"

#include <string>
#include <vector>

#include "motis/core/schedule/schedule.h"

#include "motis/protocol/RoutingResponse_generated.h"

#include "motis/reliability/distributions_calculator.h"
#include "motis/reliability/graph_accessor.h"
#include "motis/reliability/probability_distribution.h"
#include "motis/reliability/rating/connection_rating.h"

namespace motis {
namespace reliability {
namespace rating {
namespace connection_to_graph_data {

std::vector<std::vector<connection_element>> const get_elements(
    schedule const& sched, routing::Connection const* connection) {
  std::vector<std::vector<connection_element>> elements;
  for (auto it_t = connection->transports()->begin();
       it_t != connection->transports()->end(); ++it_t) {
    if (it_t->move_type() == routing::Move_Transport) {
      auto transport = (routing::Transport const*)it_t->move();
      for (auto stop_idx = transport->range()->from();
           stop_idx < transport->range()->to(); stop_idx++) {
        auto const tail_stop = (*connection->stops())[stop_idx];
        auto const head_stop = (*connection->stops())[stop_idx + 1];
        auto const element = detail::to_element(
            stop_idx, sched, tail_stop->eva_nr()->str(),
            head_stop->eva_nr()->str(),
            unix_to_motistime(sched.schedule_begin_,
                              tail_stop->departure()->time()),
            unix_to_motistime(sched.schedule_begin_,
                              head_stop->arrival()->time()),
            transport->category_name()->str(), transport->train_nr());

        // begin new train if elements empty or if there is an interchange
        if (elements.size() == 0 ||
            elements.back().back().to_->_id != element.from_->_id) {
          elements.emplace_back();
        }

        elements.back().push_back(element);
      }
    }
  }
  return elements;
}

namespace detail {

connection_element const to_element(
    unsigned int const departure_stop_idx, schedule const& sched,
    std::string const& tail_eva, std::string const& head_eva,
    motis::time const dep_time, motis::time const arr_time,
    std::string const& category_str, unsigned int const train_nr) {
  auto const& tail_station = *sched.station_nodes.at(
      sched.eva_to_station.find(tail_eva)->second->index);
  auto const head_station_id =
      sched.eva_to_station.find(head_eva)->second->index;
  unsigned int const family =
      std::find_if(sched.categories.begin(), sched.categories.end(),
                   [category_str](std::unique_ptr<category> const& cat) {
                     return cat->name == category_str;
                   }) -
      sched.categories.begin();

  for (auto it = tail_station._edges.begin(); it != tail_station._edges.end();
       it++) {
    auto const route_edge = graph_accessor::get_departing_route_edge(*it->_to);
    if (route_edge && route_edge->_to->_station_node->_id == head_station_id) {
      auto light_conn = graph_accessor::find_light_connection(
          *route_edge, dep_time, family, train_nr);
      if (light_conn.first) {
        bool const is_first_route_node =
            graph_accessor::get_arriving_route_edge(*it->_to) == nullptr;
        return connection_element(departure_stop_idx, route_edge->_from,
                                  route_edge->_to, light_conn.first,
                                  light_conn.second, is_first_route_node);
      }
    }
  }

  assert(false);
  // empty element (unexpected case)
  return connection_element(departure_stop_idx, nullptr, nullptr, nullptr, 0,
                            false);
}

}  // namespace detail
}  // namespace connection_to_graph_data
}  // namespace rating
}  // namespace reliability
}  // namespace motis
