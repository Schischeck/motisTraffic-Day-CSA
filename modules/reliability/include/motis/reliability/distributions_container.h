#pragma once

#include <cassert>
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
struct route_node_distributions {
  friend struct precomputed_distributions_container;

  probability_distribution const& get_distribution(
      unsigned int const index) const {
    assert(index < distributions_.size());
    return distributions_[index];
  }
  probability_distribution& get_distribution_non_const(
      unsigned int const index) {
    assert(index < distributions_.size());
    return distributions_[index];
  }

private:
  void init(unsigned int const size) { distributions_.resize(size); }

  std::vector<probability_distribution> distributions_;
};

struct precomputed_distributions_container {
  enum type { arrival, departure };

  precomputed_distributions_container(unsigned num_nodes)
      : node_to_departure_distributions_(num_nodes),
        node_to_arrival_distributions_(num_nodes) {}

  virtual ~precomputed_distributions_container() {}

  bool contains_departure_distributions(
      unsigned int const route_node_idx) const {
    return (bool)node_to_departure_distributions_[route_node_idx];
  }

  virtual bool contains_arrival_distributions(
      unsigned int const route_node_idx) const {
    return (bool)node_to_arrival_distributions_[route_node_idx];
  }

  virtual probability_distribution const& get_distribution(
      unsigned int const route_node_idx, unsigned int const light_conn_idx,
      type const t) const {
    if (t == arrival) {
      assert(route_node_idx < node_to_arrival_distributions_.size());
      assert(node_to_arrival_distributions_[route_node_idx]);
      return node_to_arrival_distributions_[route_node_idx]->get_distribution(
          light_conn_idx);
    } else {
      assert(route_node_idx < node_to_departure_distributions_.size());
      assert(node_to_departure_distributions_[route_node_idx]);
      return node_to_departure_distributions_[route_node_idx]->get_distribution(
          light_conn_idx);
    }
  }

  route_node_distributions& get_route_node_distributions(
      unsigned int const route_node_idx, type const t) {
    return (t == arrival) ? *node_to_arrival_distributions_[route_node_idx]
                          : *node_to_departure_distributions_[route_node_idx];
  }

  void create_route_node_distributions(unsigned int const route_node_idx,
                                  type const t, unsigned int const size) {
    if (t == departure) {
      assert(!node_to_departure_distributions_[route_node_idx]);
      node_to_departure_distributions_[route_node_idx] =
          std::unique_ptr<route_node_distributions>(new route_node_distributions);
      node_to_departure_distributions_[route_node_idx]->init(size);
    } else {
      assert(!node_to_arrival_distributions_[route_node_idx]);
      node_to_arrival_distributions_[route_node_idx] =
          std::unique_ptr<route_node_distributions>(new route_node_distributions);
      node_to_arrival_distributions_[route_node_idx]->init(size);
    }
  }

private:
  std::vector<std::unique_ptr<route_node_distributions> >
      node_to_departure_distributions_;
  std::vector<std::unique_ptr<route_node_distributions> >
      node_to_arrival_distributions_;
};

}  // namespace reliability
}  // namespace motis
