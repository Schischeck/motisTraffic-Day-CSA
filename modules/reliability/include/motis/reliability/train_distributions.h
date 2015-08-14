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
  friend struct train_distributions_container;

  probability_distribution const& get_distribution(
      unsigned int const index) const {
    return distributions_[index];
  }
  probability_distribution& get_distribution_non_const(
      unsigned int const index) {
    return distributions_[index];
  }

private:
  void init(unsigned int const size) { distributions_.resize(size); }

  std::vector<probability_distribution> distributions_;
};

struct train_distributions_container {
  enum type { arrival, departure };

  train_distributions_container(unsigned num_nodes)
      : node_to_departure_distributions_(num_nodes),
        node_to_arrival_distributions_(num_nodes) {}

  virtual ~train_distributions_container() {}

  bool contains_departure_distributions(
      unsigned int const route_node_idx) const {
    return (bool)node_to_departure_distributions_[route_node_idx];
  }

  bool contains_arrival_distributions(unsigned int const route_node_idx) const {
    return (bool)node_to_arrival_distributions_[route_node_idx];
  }

  virtual probability_distribution const& get_probability_distribution(
      unsigned int const route_node_idx, unsigned int const light_conn_idx,
      type const t) const {
    return (t == arrival)
               ? node_to_arrival_distributions_[route_node_idx]
                     ->get_distribution(light_conn_idx)
               : node_to_departure_distributions_[route_node_idx]
                     ->get_distribution(light_conn_idx);
  }

  train_distributions& get_train_distributions(
      unsigned int const route_node_idx, type const t) {
    return (t == arrival) ? *node_to_arrival_distributions_[route_node_idx]
                          : *node_to_departure_distributions_[route_node_idx];
  }

  void create_train_distributions(unsigned int const route_node_idx,
                                  type const t, unsigned int const size) {
    if (t == departure) {
      assert(!node_to_departure_distributions_[route_node_idx]);
      node_to_departure_distributions_[route_node_idx] =
          std::unique_ptr<train_distributions>(new train_distributions);
      node_to_departure_distributions_[route_node_idx]->init(size);
    } else {
      assert(!node_to_arrival_distributions_[route_node_idx]);
      node_to_arrival_distributions_[route_node_idx] =
          std::unique_ptr<train_distributions>(new train_distributions);
      node_to_arrival_distributions_[route_node_idx]->init(size);
    }
  }

private:
  std::vector<std::unique_ptr<train_distributions> >
      node_to_departure_distributions_;
  std::vector<std::unique_ptr<train_distributions> >
      node_to_arrival_distributions_;
};

}  // namespace reliability
}  // namespace motis
