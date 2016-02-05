#pragma once

#include <memory>
#include <vector>

#include "motis/core/schedule/time.h"

#include "motis/reliability/distributions/start_and_travel_distributions.h"

namespace motis {
struct light_connection;
class node;
struct schedule;
struct category;
namespace reliability {
struct probability_distribution;
namespace calc_arrival_distribution {
/**
 * struct storing all data necessary to calculate an arrival distribution.
 */
struct data_arrival {
  data_arrival(node const& departure_node, node const& arrival_node,
               light_connection const& light_connection,
               probability_distribution const& departure_distribution,
               schedule const& schedule,
               start_and_travel_distributions const& s_t_distributions);

  duration scheduled_travel_duration() const;

  struct departure_info {
    departure_info(probability_distribution const& distribution,
                   time const scheduled_departure_time)
        : distribution_(distribution),
          scheduled_departure_time_(scheduled_departure_time) {}
    probability_distribution const& distribution_;
    time const scheduled_departure_time_;
  } departure_info_;

  time scheduled_arrival_time_;

  std::vector<start_and_travel_distributions::probability_distribution_cref>
      travel_distributions_;

  /* bounds of the arrival distribution */
  int left_bound_;
  int right_bound_;

  struct {
    bool received_;
    time current_time_;
  } is_message_;

private:
  void init_arrival_time(node const&, light_connection const&, schedule const&);
  void init_travel_info(light_connection const& light_connection,
                        start_and_travel_distributions const&,
                        std::vector<std::unique_ptr<category>> const&);
};

}  // namespace calc_arrival_distribution {
}  // namespace reliability
}  // namespace motis
