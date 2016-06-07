#include "motis/reliability/realtime/realtime_update.h"

#include <set>

#include "motis/core/common/logging.h"
#include "motis/core/schedule/schedule.h"
#include "motis/core/access/trip_access.h"

#include "motis/protocol/RtUpdate_generated.h"
#include "motis/protocol/TimestampReason_generated.h"

#include "motis/reliability/computation/calc_arrival_distribution.h"
#include "motis/reliability/computation/calc_departure_distribution.h"
#include "motis/reliability/computation/data_arrival.h"
#include "motis/reliability/computation/data_departure.h"
#include "motis/reliability/context.h"
#include "motis/reliability/distributions/distributions_container.h"
#include "motis/reliability/distributions/start_and_travel_distributions.h"
#include "motis/reliability/error.h"
#include "motis/reliability/graph_accessor.h"

namespace motis {
namespace reliability {
namespace realtime {

struct lc_not_found_exception : std::exception {
  const char* what() const throw() override {
    return "Could not find the light connection for a delay-info";
  };
};

namespace graph_util {
using route_node_and_lc_info =
    std::tuple<node const*, light_connection const*, unsigned int>;

route_node_and_lc_info get_node_and_light_connection(
    node const& station, bool const is_departure,
    std::function<std::pair<light_connection const*, unsigned int>(
        edge const* route_edge)>
        find_light_conn) {
  if (is_departure) {
    for (auto const& e : station.edges_) {
      if (auto const* route_edge =
              graph_accessor::get_departing_route_edge(*e.to_)) {
        auto const light_conn = find_light_conn(route_edge);
        if (light_conn.first) {
          return std::make_tuple(route_edge->from_, light_conn.first,
                                 light_conn.second);
        }
      }
    }
  } else {
    for (auto const* e : station.incoming_edges_) {
      if (auto const* route_edge =
              graph_accessor::get_arriving_route_edge(*e->from_)) {
        auto const light_conn = find_light_conn(route_edge);
        if (light_conn.first) {
          return std::make_tuple(route_edge->to_, light_conn.first,
                                 light_conn.second);
        }
      }
    }
  }
  throw lc_not_found_exception();
}

std::pair<std::string, std::string> get_category_and_line_identification(
    TripId const* trip_id, time const graph_time, unsigned const train_id,
    schedule const& sched) {

  auto const& light_conn = find_light_conn(
      *get_trip(sched, trip_id),
      sched.eva_to_station_.at(trip_id->station_id()->str())->index_, train_id,
      trip_id->type(), graph_time);

  return std::make_pair(
      sched.categories_.at(light_conn.full_con_->con_info_->family_)->name_,
      light_conn.full_con_->con_info_->line_identifier_);
}

route_node_and_lc_info get_node_and_light_connection(
    distributions_container::container::key const& key, schedule const& sched) {
  auto const it = sched.schedule_to_delay_info_.find(schedule_event(
      key.station_index_, key.train_id_, key.type_ == time_util::departure,
      key.scheduled_event_time_));
  unsigned int const current_time = it == sched.schedule_to_delay_info_.end()
                                        ? key.scheduled_event_time_
                                        : get_event_time(it->second);

  auto find_light_conn =
      [&](edge const* route_edge) -> std::pair<light_connection const*,
                                               unsigned int> {
    auto const light_conn = graph_accessor::find_light_connection(
        *route_edge, current_time, key.type_ == time_util::departure,
        graph_accessor::find_family(sched.categories_, key.category_).second,
        key.train_id_, key.line_identifier_);
    return light_conn;
  };

  return get_node_and_light_connection(
      *sched.station_nodes_.at(key.station_index_),
      key.type_ == time_util::departure, find_light_conn);
}
}  // namespace graph_util

namespace detail {

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

struct queue_element {
  distributions_container::container::node* node_;
  node const* route_node_;
  light_connection const* lc_;
};

struct queue_element_cmp {
  bool operator()(std::unique_ptr<queue_element> const& a,
                  std::unique_ptr<queue_element> const& b) {
    if (a->node_->key_.scheduled_event_time_ ==
        b->node_->key_.scheduled_event_time_) {
      if (a->node_->key_.type_ == b->node_->key_.type_) {
        return a->node_->key_.train_id_ > b->node_->key_.train_id_;
      }
      return a->node_->key_.type_ == time_util::departure;
    }
    return a->node_->key_.scheduled_event_time_ >
           b->node_->key_.scheduled_event_time_;
  }
};
using queue_type =
    std::priority_queue<std::unique_ptr<queue_element>,
                        std::vector<std::unique_ptr<queue_element>>,
                        queue_element_cmp>;

unsigned int num_processed = 0;
unsigned int significant = 0;
unsigned int not_significant = 0;
unsigned int already_updated = 0;
unsigned int errors = 0;
void process_element(queue_type& queue,
                     std::set<distributions_container::container::node const*>&
                         currently_processed,
                     context const& c) {
  auto const& element = queue.top();
  queue.pop();

  // set containing all processed elements of the currently processed minute
  if (!currently_processed.empty() &&
      (*currently_processed.begin())->key_.scheduled_event_time_ <
          element->node_->key_.scheduled_event_time_) {
    currently_processed.clear();
  } else if (!currently_processed.insert(element->node_).second) {
    ++already_updated;
    return;
  }

  node const* route_node;
  light_connection const* lc;
  unsigned int lc_pos;

  try {
    std::tie(route_node, lc, lc_pos) =
        graph_util::get_node_and_light_connection(element->node_->key_,
                                                  c.schedule_);
  } catch (std::exception& e) {
    // LOG(logging::error) << e.what() << " key: " << element->key_;
    ++errors;
    return;
  }

  probability_distribution const pd_before = element->node_->pd_;
  if (element->node_->key_.type_ == time_util::departure) {
    calc_departure_distribution::data_departure const d_data(
        *route_node, *lc,
        graph_accessor::get_arriving_route_edge(*route_node) == nullptr,
        c.precomputed_distributions_, *element, c);
    calc_departure_distribution::compute_departure_distribution(
        d_data, element->node_->pd_);
  } else {
    calc_arrival_distribution::data_arrival const a_data(
        *graph_accessor::get_arriving_route_edge(*route_node)->from_,
        *route_node, *lc, element->node_->predecessors_.front()->pd_,
        c.schedule_, c.s_t_distributions_);
    calc_arrival_distribution::compute_arrival_distribution(
        a_data, element->node_->pd_);
  }

  if (is_significant_update(pd_before, element->node_->pd_)) {
    auto const& successors = element->node_->successors_;
    std::for_each(
        successors.begin(), successors.end(),
        [&](distributions_container::container::node* n) { queue.emplace(n); });
    ++significant;
  } else {
    ++not_significant;
  }
}

std::pair<node const*, light_connection const*> route_node_and_light_conn(
    trip const& tr, unsigned const station_idx, unsigned const train_id,
    EventType const event_type, time const graph_time)
    ->light_connection const& {
  auto const trip_edge = std::find_if(
      tr.edges_->begin(), tr.edges_->end(), [&](trip::route_edge const& e) {
        auto const light_conn =
            e.get_edge()->m_.route_edge_.conns_[tr.lcon_idx_];
        auto const s = e.route_node_->get_station()->id_;
        auto const tr = light_conn.full_con_->con_info_->train_nr_;
        auto const t = event_type == EventType_Arrival ? light_conn.a_time_
                                                       : light_conn.d_time_;
        return s == station_idx && tr == train_id && t == graph_time;

      });
  if (trip_edge == tr.edges_->end()) {
    throw std::system_error(error::failure);
  }
  return std::make_pair(
      trip_edge->route_node_,
      trip_edge->get_edge()->m_.route_edge_.conns_[tr.lcon_idx_]);
}

std::pair<node const*, light_connection const*> route_node_and_light_conn(
    trip const& tr, rt::ShiftedNode const& shifted_node,
    schedule const& sched) {
  auto const station_idx =
      sched.eva_to_station_.at(shifted_node.station_id()->str())->index_;
  return route_node_and_light_conn(
      tr, station_idx, shifted_node.trip()->train_nr(),
      (shifted_node.event_type() == rt::EventType_ARRIVAL
           ? EventType_Arrival
           : EventType_Departure),
      unix_to_motistime(sched, shifted_node.updated_time()));
}

distributions_container::container::node* find_distribution_node(
    unsigned const station_idx, light_connection const& lc,
    rt::ShiftedNode const& shifted_node, schedule const& sched,
    distributions_container::container& container) {
  auto const key = distributions_container::to_container_key(
      lc, station_idx, shifted_node.event_type() == rt::EventType_DEPARTURE
                           ? distributions_container::container::departure
                           : distributions_container::container::arrival,
      unix_to_motistime(sched, shifted_node.schedule_time()), sched);
  if (container.contains_distribution(key)) {
    return container.get_node_non_const(key);
  }
  return nullptr;
}

}  // namespace detail

void update_precomputed_distributions(
    motis::rt::RtUpdate const* res, schedule const& sched,
    start_and_travel_distributions const& s_t_distributions,
    distributions_container::container& precomputed_distributions) {
  logging::scoped_timer time("updating distributions");
  detail::num_processed = 0;
  detail::significant = 0;
  detail::not_significant = 0;
  detail::already_updated = 0;
  detail::errors = 0;

  /* add all events with updates to the queue */
  try {
    detail::queue_type queue;
    std::set<distributions_container::container::node const*>
        currently_processed;
    for (auto const& shifted_node : *res->shifted_nodes()) {
      if (shifted_node->reason() == TimestampReason_IS) {
        auto const& trip = *get_trip(sched, shifted_node->trip());
        auto const n_l =
            detail::route_node_and_light_conn(trip, *shifted_node, sched);

        if (auto n = detail::find_distribution_node(
                n_l.first->get_station()->id_, *n_l.second, *shifted_node,
                sched, precomputed_distributions)) {
          queue.emplace({n, n_l.first, n_l.second});
        }
      }
    }
  } catch (...) {
  }

  /* process all events in the queue
   * (each event adds event that depend on it into the queue) */
  context const c(sched, precomputed_distributions, s_t_distributions);
  while (!queue.empty()) {
    try {
      detail::process_element(queue, currently_processed, c);
    } catch (std::exception& e) {
      LOG(logging::error) << e.what() << std::endl;
    }
    ++detail::num_processed;
  }

  /* print statistics */
  auto to_percent = [&](unsigned int c) -> double {
    return ((c * 100.0) / static_cast<double>(detail::num_processed));
  };
  std::stringstream sst;
  sst << "Queue contained " << detail::num_processed << " elements";
  if (detail::num_processed > 0) {
    sst << "(" << std::fixed << std::setprecision(1)
        << to_percent(detail::significant) << "% significant updates, "
        << to_percent(detail::not_significant) << "% not significant updates, "
        << to_percent(detail::already_updated) << "% already updated, "
        << to_percent(detail::errors) << "% errors)";
  }
  LOG(logging::info) << sst.str();
}

}  // namespace realtime
}  // namespace reliability
}  // namespace motis
