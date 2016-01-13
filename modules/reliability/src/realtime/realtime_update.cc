#include "motis/reliability/realtime/realtime_update.h"

#include "motis/core/common/logging.h"
#include "motis/core/schedule/schedule.h"

#include "motis/protocol/RealtimeDelayInfoResponse_generated.h"

#include "motis/reliability/context.h"
#include "motis/reliability/graph_accessor.h"
#include "motis/reliability/computation/calc_arrival_distribution.h"
#include "motis/reliability/computation/calc_departure_distribution.h"
#include "motis/reliability/computation/data_arrival.h"
#include "motis/reliability/computation/data_departure.h"
#include "motis/reliability/distributions/distributions_container.h"
#include "motis/reliability/distributions/start_and_travel_distributions.h"

namespace motis {
namespace reliability {
namespace realtime {

struct lc_not_found_exception : std::exception {
  const char* what() const throw() {
    return "Could not find the light connection for a delay-info";
  };
};

namespace graph_util {
using route_node_and_lc_info =
    std::tuple<node const*, light_connection const*, unsigned int>;

route_node_and_lc_info get_node_and_light_connection(
    node const& station, bool const is_departure,
    std::function<std::pair<light_connection const*, unsigned int>(
        edge const* route_edge)> find_light_conn) {
  if (is_departure) {
    for (auto const& e : station._edges) {
      if (auto const* route_edge =
              graph_accessor::get_departing_route_edge(*e._to)) {
        auto const light_conn = find_light_conn(route_edge);
        if (light_conn.first) {
          return std::make_tuple(route_edge->_from, light_conn.first,
                                 light_conn.second);
        }
      }
    }
  } else {
    for (auto const* e : station._incoming_edges) {
      if (auto const* route_edge =
              graph_accessor::get_arriving_route_edge(*e->_from)) {
        auto const light_conn = find_light_conn(route_edge);
        if (light_conn.first) {
          return std::make_tuple(route_edge->_to, light_conn.first,
                                 light_conn.second);
        }
      }
    }
  }
  throw lc_not_found_exception();
}

unsigned int get_event_time(motis::realtime::DelayInfo const* delay_info) {
  return delay_info->reason() ==
                 motis::realtime::InternalTimestampReason_Forecast
             ? delay_info->forecast_time()
             : delay_info->current_time();
}

std::pair<std::string, std::string> get_category_and_line_identification(
    motis::realtime::DelayInfo const* delay_info, schedule const& sched) {
  auto find_light_conn = [&](edge const* route_edge)
      -> std::pair<light_connection const*, unsigned int> {
        auto const light_conn = graph_accessor::find_light_connection(
            *route_edge, get_event_time(delay_info),
            delay_info->departure() == 1, [&](connection_info const& ci) {
              return ci.train_nr == delay_info->train_nr();
            });
        return light_conn;
      };

  auto res = get_node_and_light_connection(
      *sched.station_nodes.at(delay_info->station_index()),
      delay_info->departure() == 1, find_light_conn);

  auto const* light_conn = std::get<1>(res);
  return std::make_pair(
      sched.categories.at(light_conn->_full_con->con_info->family)->name,
      light_conn->_full_con->con_info->line_identifier);
}

route_node_and_lc_info get_node_and_light_connection(
    distributions_container::container::key const& key, schedule const& sched) {
  auto get_event_time = [](motis::delay_info const* delay_info) {
    return delay_info->_reason == motis::timestamp_reason::FORECAST
               ? delay_info->_forecast_time
               : delay_info->_current_time;
  };

  auto const it = sched.schedule_to_delay_info.find(schedule_event(
      key.station_index_, key.train_id_, key.type_ == time_util::departure,
      key.scheduled_event_time_));
  unsigned int const current_time = it == sched.schedule_to_delay_info.end()
                                        ? key.scheduled_event_time_
                                        : get_event_time(it->second);

  auto find_light_conn = [&](edge const* route_edge)
      -> std::pair<light_connection const*, unsigned int> {
        auto const light_conn = graph_accessor::find_light_connection(
            *route_edge, current_time, key.type_ == time_util::departure,
            graph_accessor::find_family(sched.categories, key.category_).second,
            key.train_id_, key.line_identifier_);
        return light_conn;
      };

  return get_node_and_light_connection(
      *sched.station_nodes.at(key.station_index_),
      key.type_ == time_util::departure, find_light_conn);
}
}

namespace detail {
/* update the distribution of the node and deliver dependent successors */
std::vector<distributions_container::container::node*> const&
process_is_message(
    motis::realtime::DelayInfo const* delay_info, schedule const& sched,
    distributions_container::container& precomputed_distributions) {
  // TODO: category and line-identification can be read from the RIS-message
  auto const cat_line =
      graph_util::get_category_and_line_identification(delay_info, sched);
  distributions_container::container::key key(
      delay_info->train_nr(), cat_line.first, cat_line.second,
      delay_info->station_index(),
      delay_info->departure() == 1 ? time_util::departure : time_util::arrival,
      delay_info->scheduled_time());

  auto& dist_node = precomputed_distributions.get_node_non_const(key);
  int delay = (int)graph_util::get_event_time(delay_info) -
              (int)delay_info->scheduled_time();
  if (delay_info->departure() == 1 && delay < 0) {
    delay = 0;
  }
  dist_node.pd_.init_one_point(delay, 1.0);
  return dist_node.successors_;
}

bool is_significant_update(probability_distribution const& before,
                           probability_distribution const& after) {
  int const first_min = std::min(before.first_minute(), after.first_minute());
  int const last_min = std::min(before.last_minute(), after.last_minute());
  double error = 0.0;
  for (int min = first_min; min <= last_min; ++min) {
    error +=
        std::abs(before.probability_equal(min) - after.probability_equal(min));
    if (error > 0.001) {
      return true;
    }
  }
  return false;
}

struct queue_element_cmp {
  bool operator()(distributions_container::container::node const* a,
                  distributions_container::container::node const* b) {
    if (a->key_.scheduled_event_time_ == b->key_.scheduled_event_time_) {
      if (a->key_.type_ == b->key_.type_) {
        return a->key_.train_id_ > b->key_.train_id_;
      }
      return a->key_.type_ == time_util::departure;
    }
    return a->key_.scheduled_event_time_ > b->key_.scheduled_event_time_;
  }
};
using queue_type =
    std::priority_queue<distributions_container::container::node*,
                        std::vector<distributions_container::container::node*>,
                        queue_element_cmp>;

unsigned int not_significant = 0;
unsigned int ignored_elements = 0;
void process_element(queue_type& queue, context const& c) {
  auto* element = queue.top();
  queue.pop();
  node const* route_node;
  light_connection const* lc;
  unsigned int lc_pos;

  try {
    std::tie(route_node, lc, lc_pos) =
        graph_util::get_node_and_light_connection(element->key_, c.schedule_);
  } catch (std::exception& e) {
    // LOG(logging::error) << e.what() << " key: " << element->key_;
    ++ignored_elements;
    return;
  }

  probability_distribution const pd_before = element->pd_;
  if (element->key_.type_ == time_util::departure) {
    calc_departure_distribution::data_departure const d_data(
        *route_node, *lc,
        graph_accessor::get_arriving_route_edge(*route_node) == nullptr,
        c.precomputed_distributions_, *element, c);
    calc_departure_distribution::compute_departure_distribution(d_data,
                                                                element->pd_);
  } else {
    calc_arrival_distribution::data_arrival const a_data(
        *graph_accessor::get_arriving_route_edge(*route_node)->_from,
        *route_node, *lc, element->predecessors_.front()->pd_, c.schedule_,
        c.s_t_distributions_);
    calc_arrival_distribution::compute_arrival_distribution(a_data,
                                                            element->pd_);
  }

  if (is_significant_update(pd_before, element->pd_)) {
    auto const& successors = element->successors_;
    std::for_each(
        successors.begin(), successors.end(),
        [&](distributions_container::container::node* n) { queue.emplace(n); });
  } else {
    ++not_significant;
  }
}
}

void update_precomputed_distributions(
    motis::realtime::RealtimeDelayInfoResponse const* res,
    schedule const& sched,
    start_and_travel_distributions const& s_t_distributions,
    distributions_container::container& precomputed_distributions) {
  logging::scoped_timer time("updating distributions");
  detail::queue_type queue;
  for (auto it = res->delay_infos()->begin(); it != res->delay_infos()->end();
       ++it) {
    if (it->reason() == motis::realtime::InternalTimestampReason_Is) {
      try {
        auto const& successors =
            detail::process_is_message(*it, sched, precomputed_distributions);
        std::for_each(successors.begin(), successors.end(),
                      [&](distributions_container::container::node* n) {
                        queue.emplace(n);
                      });
      } catch (std::exception& e) {
        /*LOG(logging::error) << e.what() << " tr: " << it->train_nr()
                            << " st: " << it->station_index();*/
      }
    }
  }

  unsigned int num_processed = 0;
  context const c(sched, precomputed_distributions, s_t_distributions);
  while (!queue.empty()) {
    try {
      detail::process_element(queue, c);
    } catch (std::exception& e) {
      LOG(logging::error) << e.what() << std::endl;
    }
    ++num_processed;
  }
  LOG(logging::info) << "Distributions updated " << num_processed
                     << " distributions (not significant updates: "
                     << detail::not_significant
                     << ", ignored elements: " << detail::ignored_elements
                     << ")";
}

}  // namespace realtime
}  // namespace reliability
}  // namespace motis
