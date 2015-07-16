#pragma once

#include <memory>
#include <vector>

#include "motis/reliability/probability_distribution.h"

namespace motis {
namespace reliability {

/**
 * Each route-node has a train_distributions object
 * that contains all arrival and departure distributions
 * of its arriving and departing light connections.
 */
struct train_distributions {

  train_distributions(unsigned int const num_arrival_distributions,
                      unsigned int const num_departure_distributions)
      : arrival_distributions_(num_arrival_distributions,
                               probability_distribution()),
        departure_distributions_(num_departure_distributions,
                                 probability_distribution()) {}

  std::vector<probability_distribution> arrival_distributions_;
  std::vector<probability_distribution> departure_distributions_;
};

struct train_distributions_container {
  enum type { arrival, departure };

  train_distributions_container(unsigned num_nodes)
      : node_to_train_distributions_(num_nodes) {}

  probability_distribution const& get_train_distribution(
      unsigned int const route_node_idx, unsigned int const light_conn_idx,
      type const t) const {
    return (t == arrival)
               ? node_to_train_distributions_[route_node_idx]
                     ->arrival_distributions_[light_conn_idx]
               : node_to_train_distributions_[route_node_idx]
                     ->departure_distributions_[light_conn_idx];
  }

private:
  std::vector<std::unique_ptr<train_distributions> >
      node_to_train_distributions_;
};

}  // namespace reliability
}  // namespace motis
