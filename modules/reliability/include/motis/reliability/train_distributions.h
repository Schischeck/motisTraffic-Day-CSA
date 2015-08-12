#pragma once

#include <memory>
#include <vector>

#include "motis/reliability/probability_distribution.h"

namespace motis {
namespace reliability {

struct train_distributions_calculator;

/**
 * Each route-node has a train_distributions object
 * that contains all arrival and departure distributions
 * of its arriving and departing light connections.
 */
struct train_distributions {
  friend struct train_distributions_calculator;

  train_distributions(unsigned int const num_arrival_distributions,
                      unsigned int const num_departure_distributions)
      : arrival_distributions_(num_arrival_distributions,
                               probability_distribution()),
        departure_distributions_(num_departure_distributions,
                                 probability_distribution()) {}

  probability_distribution const& get_arrival_distribution(
      unsigned int const index) {
    return arrival_distributions_[index];
  }

  probability_distribution const& get_departure_distribution(
      unsigned int const index) {
    return departure_distributions_[index];
  }

private:
  std::vector<probability_distribution> arrival_distributions_;
  std::vector<probability_distribution> departure_distributions_;
};

struct train_distributions_container {
  enum type { arrival, departure };

  train_distributions_container(unsigned num_nodes)
      : node_to_train_distributions_(num_nodes) {}

  virtual ~train_distributions_container() {}

  void init_route_node(unsigned int const route_node_idx,
                       unsigned int const num_arrival_distributions,
                       unsigned int const num_departure_distributions) {
    node_to_train_distributions_[route_node_idx] =
        std::unique_ptr<train_distributions>(new train_distributions(
            num_arrival_distributions, num_departure_distributions));
  }

  bool contains_route_node(unsigned int const route_node_idx) {
    return (bool)node_to_train_distributions_[route_node_idx];
  }

  virtual probability_distribution const& get_train_distribution(
      unsigned int const route_node_idx, unsigned int const light_conn_idx,
      type const t) const {
    return (t == arrival)
               ? node_to_train_distributions_[route_node_idx]
                     ->get_arrival_distribution(light_conn_idx)
               : node_to_train_distributions_[route_node_idx]
                     ->get_departure_distribution(light_conn_idx);
  }

  train_distributions get_train_distributions(
      unsigned int const route_node_idx) {
    return *node_to_train_distributions_[route_node_idx];
  }

private:
  std::vector<std::unique_ptr<train_distributions> >
      node_to_train_distributions_;
};

}  // namespace reliability
}  // namespace motis
