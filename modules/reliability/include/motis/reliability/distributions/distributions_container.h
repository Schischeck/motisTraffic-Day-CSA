#pragma once

#include <cassert>
#include <map>
#include <memory>
#include <tuple>
#include <vector>

#include "motis/reliability/distributions/probability_distribution.h"

namespace motis {
namespace reliability {
namespace distributions_container {
enum event_type { arrival, departure };

struct abstract_distributions_container {
  virtual ~abstract_distributions_container() {}
  virtual probability_distribution const& get_distribution(
      unsigned int const route_node_idx, unsigned int const light_conn_idx,
      event_type const t) const = 0;
};

struct precomputed_distributions_container : abstract_distributions_container {
  precomputed_distributions_container(unsigned num_nodes)
      : node_to_departure_distributions_(num_nodes),
        node_to_arrival_distributions_(num_nodes) {}

  virtual ~precomputed_distributions_container() {}

  virtual bool contains_distributions(unsigned int const route_node_idx,
                                      event_type const t) const {
    return (t == departure
                ? node_to_departure_distributions_.at(route_node_idx).size() > 0
                : node_to_arrival_distributions_.at(route_node_idx).size() > 0);
  }

  virtual probability_distribution const& get_distribution(
      unsigned int const route_node_idx, unsigned int const light_conn_idx,
      event_type const t) const override {
    assert(contains_distributions(route_node_idx, t));
    auto const& distribution =
        (t == departure
             ? node_to_departure_distributions_.at(route_node_idx)
                   .at(light_conn_idx)
             : node_to_arrival_distributions_.at(route_node_idx)
                   .at(light_conn_idx));
    assert(!distribution.empty());
    return distribution;
  }

  probability_distribution& get_distribution_non_const(
      unsigned int const route_node_idx, unsigned int const light_conn_idx,
      event_type const t) {
    assert(contains_distributions(route_node_idx, t));
    auto& distribution =
        (t == departure
             ? node_to_departure_distributions_.at(route_node_idx)
                   .at(light_conn_idx)
             : node_to_arrival_distributions_.at(route_node_idx)
                   .at(light_conn_idx));
    // assert(distribution.empty());
    return distribution;
  }

  void create_route_node_distributions(unsigned int const route_node_idx,
                                       event_type const t,
                                       unsigned int const size) {
    auto& vec =
        (t == departure ? node_to_departure_distributions_.at(route_node_idx)
                        : node_to_arrival_distributions_.at(route_node_idx));
    assert(vec.size() == 0);
    vec.resize(size);
  }

private:
  /* two dimensional vector [route-node-id][light-conn-idx] */
  std::vector<std::vector<probability_distribution> >
      node_to_departure_distributions_;
  /* two dimensional vector [route-node-id][light-conn-idx] */
  std::vector<std::vector<probability_distribution> >
      node_to_arrival_distributions_;
};  // struct precomputed_distributions_container

struct ride_distributions_container : abstract_distributions_container {
  probability_distribution const& get_distribution(
      unsigned int const route_node_idx, unsigned int const light_conn_idx,
      event_type const t) const override {
    auto const it =
        distributions_.find(std::make_tuple(route_node_idx, light_conn_idx, t));
    assert(it != distributions_.end());
    return it->second;
  }

  probability_distribution& create_and_get_distribution_non_const(
      unsigned int const route_node_idx, unsigned int const light_conn_idx,
      event_type const t) {
    auto key = std::make_tuple(route_node_idx, light_conn_idx, t);
    assert(distributions_.find(key) == distributions_.end());
    auto& pd = distributions_[key];
    return pd;
  }

private:
  /* key: route_node_idx, light_conn_idx, type */
  std::map<std::tuple<unsigned int, unsigned int, unsigned int>,
           probability_distribution> distributions_;
};  // struct ride_distributions_container

struct single_distribution_container : abstract_distributions_container {
  single_distribution_container(probability_distribution const& distribution)
      : distribution_(distribution) {}
  probability_distribution const& get_distribution(
      unsigned int const, unsigned int const, event_type const) const override {
    return distribution_;
  };

private:
  probability_distribution const& distribution_;
};

}  // namespace distributions_container
}  // namespace reliability
}  // namespace motis
