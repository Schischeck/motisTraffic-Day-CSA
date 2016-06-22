#include "motis/reliability/realtime/realtime_update.h"

#include <memory>
#include <set>

#include "motis/core/common/logging.h"
#include "motis/core/schedule/schedule.h"
#include "motis/core/access/trip_access.h"
#include "../../include/motis/reliability/realtime/graph_access.h"

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
#include "motis/reliability/realtime/graph_access.h"

namespace motis {
namespace reliability {
namespace realtime {

struct lc_not_found_exception : std::exception {
  const char* what() const throw() override {
    return "Could not find the light connection for a delay-info";
  };
};

namespace detail {

struct stat {
  stat()
      : num_processed_(0),
        significant_(0),
        not_significant_(0),
        already_updated_(0),
        errors_(0),
        shifted_nodes_(0),
        trip_not_found_(0),
        graph_event_not_found_(0),
        invalid_lc_(0) {}

  void print(std::ostream& os) {
    auto to_percent = [](unsigned int c, unsigned int total) -> double {
      return ((c * 100.0) / static_cast<double>(total));
    };
    auto p = [&](unsigned int c) -> double {
      return to_percent(c, num_processed_);
    };
    os << "\nDistributions updated\n"
       << std::setw(10) << num_processed_ << " elements added to the queue\n";
    if (num_processed_ > 0) {
      os << std::fixed << std::setprecision(1) << std::setw(9)
         << p(significant_) << "% significant updates\n"
         << std::setw(9) << p(not_significant_) << "% not significant updates\n"
         << std::setw(9) << p(already_updated_) << "% already updated\n"
         << std::setw(9) << p(invalid_lc_) << "% invalid light-conn\n"
         << std::setw(10) << errors_ << " errors\n"
         << std::setw(10) << shifted_nodes_ << " shifted nodes\n"
         << std::setw(10) << trip_not_found_ << " trips not found\n"
         << std::setw(10) << graph_event_not_found_
         << " graph-events not found\n";
    }
  }

  uint64_t num_processed_, significant_, not_significant_, already_updated_,
      errors_, shifted_nodes_, trip_not_found_, graph_event_not_found_,
      invalid_lc_;
};

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
  bool operator()(queue_element const& a, queue_element const& b) {
    if (a.node_->key_.scheduled_event_time_ ==
        b.node_->key_.scheduled_event_time_) {
      if (a.node_->key_.type_ == b.node_->key_.type_) {
        return a.node_->key_.train_id_ > b.node_->key_.train_id_;
      }
      return a.node_->key_.type_ == event_type::DEP;
    }
    return a.node_->key_.scheduled_event_time_ >
           b.node_->key_.scheduled_event_time_;
  }
};
using queue_type =
    std::priority_queue<queue_element, std::vector<queue_element>,
                        queue_element_cmp>;

void process_element(queue_type& queue,
                     std::set<distributions_container::container::node const*>&
                         currently_processed,
                     context const& c, detail::stat& stat) {
  auto const element = queue.top();
  queue.pop();

  // set containing all processed elements of the currently processed minute
  if (!currently_processed.empty() &&
      (*currently_processed.begin())->key_.scheduled_event_time_ <
          element.node_->key_.scheduled_event_time_) {
    currently_processed.clear();
  } else if (!currently_processed.insert(element.node_).second) {
    ++stat.already_updated_;
    return;
  }

  if (element.lc_->valid_ == 0) {
    ++stat.invalid_lc_;
    return;
  }

  probability_distribution const pd_before = element.node_->pd_;
  if (element.node_->key_.type_ == event_type::DEP) {
    calc_departure_distribution::data_departure const d_data(
        *element.route_node_, *element.lc_,
        graph_accessor::get_arriving_route_edge(*element.route_node_) ==
            nullptr,
        c.precomputed_distributions_, *element.node_, c);
    calc_departure_distribution::compute_departure_distribution(
        d_data, element.node_->pd_);
  } else {
    calc_arrival_distribution::data_arrival const a_data(
        *graph_accessor::get_arriving_route_edge(*element.route_node_)->from_,
        *element.route_node_, *element.lc_,
        element.node_->predecessors_.front()->pd_, c.schedule_,
        c.s_t_distributions_);
    calc_arrival_distribution::compute_arrival_distribution(a_data,
                                                            element.node_->pd_);
  }

  if (is_significant_update(pd_before, element.node_->pd_)) {
    auto const& successors = element.node_->successors_;
    std::for_each(successors.begin(), successors.end(),
                  [&](distributions_container::container::node* n) {
                    try {
                      auto const n_l =
                          graph_access::get_node_and_light_connection(
                              n->key_, c.schedule_);
                      queue.push({n, n_l.first, n_l.second});
                    } catch (...) {
                      ++stat.graph_event_not_found_;
                    }
                  });
    ++stat.significant_;
  } else {
    ++stat.not_significant_;
  }
}

auto route_node_and_light_conn(trip const& tr, unsigned const station_idx,
                               unsigned const train_id,
                               EventType const event_type,
                               time const graph_time) {
  auto const trip_edge = std::find_if(
      tr.edges_->begin(), tr.edges_->end(), [&](trip::route_edge const& e) {
        auto const re = e.get_edge();
        auto const light_conn = re->m_.route_edge_.conns_[tr.lcon_idx_];
        auto const s = event_type == EventType_ARR
                           ? re->to_->get_station()->id_
                           : re->from_->get_station()->id_;
        auto const tr = light_conn.full_con_->con_info_->train_nr_;
        auto const t = event_type == EventType_ARR ? light_conn.a_time_
                                                   : light_conn.d_time_;
        return s == station_idx && tr == train_id && t == graph_time;

      });
  if (trip_edge == tr.edges_->end()) {
    throw std::system_error(error::failure);
  }
  return std::make_pair(
      trip_edge->route_node_,
      &trip_edge->get_edge()->m_.route_edge_.conns_[tr.lcon_idx_]);
}

auto route_node_and_light_conn(trip const& tr,
                               rt::ShiftedNode const& shifted_node,
                               schedule const& sched) {
  auto const station_idx =
      sched.eva_to_station_.at(shifted_node.station_id()->str())->index_;
  return route_node_and_light_conn(
      tr, station_idx, shifted_node.trip()->train_nr(),
      shifted_node.event_type(),
      unix_to_motistime(sched, shifted_node.updated_time()));
}

distributions_container::container::node* find_distribution_node(
    unsigned const station_idx, light_connection const& lc,
    rt::ShiftedNode const& shifted_node, schedule const& sched,
    distributions_container::container& container) {
  auto const key = distributions_container::to_container_key(
      lc, station_idx, from_fbs(shifted_node.event_type()),
      unix_to_motistime(sched, shifted_node.schedule_time()), sched);
  if (container.contains_distribution(key)) {
    return &container.get_node_non_const(key);
  }
  return nullptr;
}

}  // namespace detail

void update_precomputed_distributions(
    motis::rt::RtUpdate const& res, schedule const& sched,
    start_and_travel_distributions const& s_t_distributions,
    distributions_container::container& precomputed_distributions) {
  logging::scoped_timer time("updating distributions");
  detail::stat stat;

  /* add all events with updates to the queue */
  detail::queue_type queue;
  std::set<distributions_container::container::node const*> currently_processed;
  for (auto const& shifted_node : *res.shifted_nodes()) {
    if (shifted_node->reason() == TimestampReason_IS) {
      ++stat.shifted_nodes_;
      try {
        auto const& trip = *get_trip(sched, shifted_node->trip());
        auto const n_l =
            detail::route_node_and_light_conn(trip, *shifted_node, sched);

        if (auto n = detail::find_distribution_node(
                n_l.first->get_station()->id_, *n_l.second, *shifted_node,
                sched, precomputed_distributions)) {
          queue.push({n, n_l.first, n_l.second});
        }
      } catch (...) {
        ++stat.trip_not_found_;
      }
    }
  }

  /* process all events in the queue
   * (each event adds event that depend on it into the queue) */
  context const c(sched, precomputed_distributions, s_t_distributions);
  while (!queue.empty()) {
    ++stat.num_processed_;
    try {
      detail::process_element(queue, currently_processed, c, stat);
    } catch (std::exception& e) {
      LOG(logging::error) << e.what() << std::endl;
      ++stat.errors_;
    }
  }

  stat.print(LOG(logging::info));
}

}  // namespace realtime
}  // namespace reliability
}  // namespace motis
