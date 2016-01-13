#include "motis/reliability/computation/distributions_calculator.h"

#include <fstream>

#include "motis/core/schedule/schedule.h"
#include "motis/core/common/logging.h"

#include "motis/reliability/context.h"
#include "motis/reliability/distributions/distributions_container.h"
#include "motis/reliability/graph_accessor.h"
#include "motis/reliability/computation/calc_arrival_distribution.h"
#include "motis/reliability/computation/calc_departure_distribution.h"
#include "motis/reliability/computation/data_arrival.h"
#include "motis/reliability/computation/data_departure.h"
#include "motis/reliability/rating/connection_to_graph_data.h"

namespace motis {
namespace reliability {
namespace distributions_calculator {

namespace common {
void output_element(std::ostream& os, schedule const& schedule,
                    node const& from, node const& to,
                    light_connection const& light_connection,
                    unsigned short const light_connection_idx,
                    bool const is_first_route_node) {
  os << schedule.stations[from._station_node->_id]->name << "("
     << schedule.stations[from._station_node->_id]->index << "," << from._id
     << ") " << format_time(light_connection.d_time) << "--"
     << schedule.categories[light_connection._full_con->con_info->family]->name
     << light_connection._full_con->con_info->train_nr << "("
     << light_connection_idx << ")->" << format_time(light_connection.a_time)
     << " " << schedule.stations[to._station_node->_id]->name << "(" << to._id
     << ")"
     << " first=" << is_first_route_node << " " << &light_connection
     << std::endl;
}
void output_element(std::ostream& os, schedule const& schedule,
                    queue_element const& element) {
  output_element(os, schedule, *element.from_, *element.to_,
                 *element.light_connection_, element.light_connection_idx_,
                 element.is_first_route_node_);
}

void compute_dep_and_arr_distribution(
    queue_element const& element,
    distributions_container::container::node const& departing_distribution_node,
    probability_distribution& departure_distribution,
    probability_distribution& arrival_distribution, context const& context,
    distributions_container::container const& train_distributions_container) {
  calc_departure_distribution::data_departure d_data(
      *element.from_, *element.light_connection_, element.is_first_route_node_,
      train_distributions_container, departing_distribution_node, context);
  calc_departure_distribution::compute_departure_distribution(
      d_data, departure_distribution);

  calc_arrival_distribution::data_arrival a_data(
      *element.from_, *element.to_, *element.light_connection_,
      departure_distribution, context.schedule_, context.s_t_distributions_);
  calc_arrival_distribution::compute_arrival_distribution(a_data,
                                                          arrival_distribution);
}
}  // namespace common

namespace precomputation {
namespace detail {
/* returns success, begin_time, end_time */
std::tuple<bool, time, time> get_feeder_time_interval(
    time const departure_time, duration change_time,
    duration const feeder_threshold) {
  change_time = std::max(change_time, (duration)1);
  time const begin_time =
      (departure_time <= feeder_threshold ? 0
                                          : departure_time - feeder_threshold);
  time const end_time = departure_time - change_time;
  bool const success =
      (begin_time <= end_time && departure_time >= change_time);

  return std::make_tuple(success, begin_time, end_time);
}

#define FEEDER_THRESHOLD 30 /* XXX */
std::vector<std::pair<node const*, light_connection const*>> get_feeders(
    node const& route_node, light_connection const& light_conn,
    schedule const& sched) {
  duration const transfer_time =
      sched.stations.at(route_node.get_station()->_id)->transfer_time;
  bool success;
  time time_begin, time_end;
  std::tie(success, time_begin, time_end) = get_feeder_time_interval(
      light_conn.d_time, transfer_time, FEEDER_THRESHOLD);
  if (!success) {
    return {};
  }
  auto is_feeder = [&](light_connection const& feeder_light_conn) -> bool {
    return feeder_light_conn.a_time >= time_begin &&
           feeder_light_conn.a_time <= time_end &&
           graph_accessor::get_waiting_time(sched.waiting_time_rules_,
                                            feeder_light_conn, light_conn) > 0;
  };

  std::vector<std::pair<node const*, light_connection const*>> feeders;
  for (auto const in_edge : route_node._station_node->_incoming_edges) {
    auto const& feeder_route_node = *in_edge->_from;
    auto const feeder_route_edge =
        graph_accessor::get_arriving_route_edge(feeder_route_node);
    if (feeder_route_edge == nullptr ||
        feeder_route_node._id == route_node._id) {
      continue;
    }
    auto& all_connections = feeder_route_edge->_m._route_edge._conns;
    for (unsigned int i = 0; i < all_connections.size(); i++) {
      auto const& feeder_light_conn = all_connections[i];
      if (is_feeder(feeder_light_conn)) {
        feeders.emplace_back(&feeder_route_node, &feeder_light_conn);
      }
    }  // end of for all_connections
  }  // end of for _incoming_edges

  return feeders;
}

void add_feeders(common::queue_element const& element,
                 distributions_container::container::node& departure_dist_node,
                 distributions_container::container& container,
                 schedule const& sched) {
  auto const feeders =
      detail::get_feeders(*element.from_, *element.light_connection_, sched);
  for (auto const& f : feeders) {
    auto& feeder_node =
        container.get_node_non_const(distributions_container::to_container_key(
            *f.first, *f.second, time_util::arrival, sched));
    departure_dist_node.predecessors_.push_back(&feeder_node);
    feeder_node.successors_.push_back(&departure_dist_node);
  }
}

void init_predecessors_and_successors(
    distributions_container::container::node& departure_distribution_node,
    distributions_container::container::node& arrival_distribution_node,
    distributions_container::container& distributions_container,
    common::queue_element const& element, schedule const& schedule) {
  departure_distribution_node.successors_.push_back(&arrival_distribution_node);
  arrival_distribution_node.predecessors_.push_back(
      &departure_distribution_node);
  add_feeders(element, departure_distribution_node, distributions_container,
              schedule);
  if (auto const* route_edge =
          graph_accessor::get_arriving_route_edge(*element.from_)) {
    auto& preceding_arrival_distribution_node =
        distributions_container.get_node_non_const(
            distributions_container::to_container_key(
                *element.from_, graph_accessor::get_previous_light_connection(
                                    route_edge->_m._route_edge._conns,
                                    element.light_connection_->d_time),
                time_util::arrival, schedule));
    preceding_arrival_distribution_node.successors_.push_back(
        &departure_distribution_node);
    departure_distribution_node.predecessors_.push_back(
        &preceding_arrival_distribution_node);
  }
}

bool is_pre_computed_route(schedule const& schedule,
                           node const& first_route_node) {
  node const* node = &first_route_node;
  edge const* route_edge = nullptr;
  while ((route_edge = graph_accessor::get_departing_route_edge(*node)) !=
         nullptr) {
    for (unsigned i = 0; i < route_edge->_m._route_edge._conns.size(); i++) {
      auto const family =
          route_edge->_m._route_edge._conns[i]._full_con->con_info->family;
      if (schedule.waiting_time_rules_.other_trains_wait_for(
              schedule.waiting_time_rules_.waiting_time_category(family)) ||
          /* trains which wait for other trains are added to the
           * precomputed-distributions-container since
           * this is the only way to find their feeders
           * (see container::node). */
          schedule.waiting_time_rules_.waits_for_other_trains(
              schedule.waiting_time_rules_.waiting_time_category(family))) {
        return true;
      }
    }
    node = route_edge->_to;
  }
  return false;
}

template <bool IsFirstRouteNode, bool CheckClass>
void insert_all_light_connections(node const& tail_node,
                                  common::queue_type& queue,
                                  schedule const& schedule) {
  if (!CheckClass || detail::is_pre_computed_route(schedule, tail_node)) {
    auto const route_edge = graph_accessor::get_departing_route_edge(tail_node);
    if (route_edge == nullptr) {
      return;
    }
    for (unsigned short light_conn_idx = 0;
         light_conn_idx < route_edge->_m._route_edge._conns.size();
         ++light_conn_idx) {
      auto& light_conn = route_edge->_m._route_edge._conns[light_conn_idx];
      queue.emplace(&tail_node, route_edge->_to, &light_conn, light_conn_idx,
                    IsFirstRouteNode);
    }
  }
}

void process_element(
    common::queue_element const& element, schedule const& schedule,
    start_and_travel_distributions const& s_t_distributions,
    common::queue_type& queue,
    distributions_container::container& distributions_container) {
  /* departure distribution */
  auto& departure_distribution_node =
      distributions_container.get_node_non_const(
          distributions_container::to_container_key(
              *element.from_, *element.light_connection_, time_util::departure,
              schedule));
  /* arrival distribution */
  auto& arrival_distribution_node = distributions_container.get_node_non_const(
      distributions_container::to_container_key(*element.to_,
                                                *element.light_connection_,
                                                time_util::arrival, schedule));
  if (!departure_distribution_node.pd_.empty() ||
      !arrival_distribution_node.pd_.empty()) {
    std::cout << "\nWarning(distributions_calculator): departure or arrival "
                 "distribution already computed: ";
    common::output_element(std::cout, schedule, element);
    return;
  }

  init_predecessors_and_successors(departure_distribution_node,
                                   arrival_distribution_node,
                                   distributions_container, element, schedule);

  common::compute_dep_and_arr_distribution(
      element, departure_distribution_node, departure_distribution_node.pd_,
      arrival_distribution_node.pd_,
      context(schedule, distributions_container, s_t_distributions),
      distributions_container);
  // insert all light connections out-going from the head-node
  // into the queue. Note that, process element is called
  // for all light-connections of the route-edge.
  // But we have to insert the light connections of the head-node
  // only once into the queue.
  if (element.light_connection_idx_ == 0) {
    detail::insert_all_light_connections<false, false>(*element.to_, queue,
                                                       schedule);
  }
}
}  // namespace detail

void perform_precomputation(
    schedule const& schedule,
    start_and_travel_distributions const& s_t_distributions,
    distributions_container::container& distributions_container) {
  logging::scoped_timer time("computing distributions");
  common::queue_type queue;

  for (auto const first_route_node : schedule.route_index_to_first_route_node) {
    detail::insert_all_light_connections<true, true>(*first_route_node, queue,
                                                     schedule);
  }

  unsigned int num_distributions = 0;
  while (!queue.empty()) {
    detail::process_element(queue.top(), schedule, s_t_distributions, queue,
                            distributions_container);
    queue.pop();
    ++num_distributions;
  }
  LOG(logging::info) << "precomputed distributions: " << num_distributions;
}

}  // namespace precomputation
}  // namespace distributions_calculator
}  // namespace reliability
}  // namespace motis
