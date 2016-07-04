#pragma once

#include <cstdlib>
#include <cstring>
#include <memory>
#include <vector>

#include "motis/core/common/array.h"
#include "motis/core/schedule/edges.h"
#include "motis/core/schedule/time.h"

namespace motis {

class station_node;
class node;
class label;

enum class node_type { STATION_NODE, ROUTE_NODE, FOOT_NODE };

class node {
public:
  node(station_node* station_node, int node_id)
      : station_node_(station_node), route_(-1), id_(node_id) {}

  bool is_station_node() const { return station_node_ == nullptr; }
  bool is_route_node() const { return route_ != -1; }
  bool is_foot_node() const { return !is_station_node() && !is_route_node(); }

  node_type type() const {
    if (is_station_node()) {
      return node_type::STATION_NODE;
    } else if (is_route_node()) {
      return node_type::ROUTE_NODE;
    } else {
      return node_type::FOOT_NODE;
    }
  }

  char const* type_str() const {
    if (is_station_node()) {
      return "STATION_NODE";
    } else if (is_route_node()) {
      return "ROUTE_NODE";
    } else {
      return "FOOT_NODE";
    }
  }

  station_node* as_station_node() {
    if (station_node_ == nullptr) {
      return reinterpret_cast<station_node*>(this);
    } else {
      return nullptr;
    }
  }

  station_node const* as_station_node() const {
    if (station_node_ == nullptr) {
      return reinterpret_cast<station_node const*>(this);
    } else {
      return nullptr;
    }
  }

  station_node* get_station() {
    if (station_node_ == nullptr) {
      return reinterpret_cast<station_node*>(this);
    } else {
      return station_node_;
    }
  }

  station_node const* get_station() const {
    if (station_node_ == nullptr) {
      return reinterpret_cast<station_node const*>(this);
    } else {
      return station_node_;
    }
  }

  array<edge> edges_;
  array<edge*> incoming_edges_;
  station_node* station_node_;
  int32_t route_;
  uint32_t id_;
};

class station_node : public node {
public:
  explicit station_node(int node_id)
      : node(nullptr, node_id), foot_node_(nullptr) {}

  ~station_node() {
    for (auto& route_node : get_route_nodes()) {
      delete route_node;
    }

    if (foot_node_ != nullptr) {
      delete foot_node_;
    }
  }

  std::vector<node const*> get_route_nodes() const {
    std::vector<node const*> route_nodes;

    for (auto& edge : edges_) {
      if (edge.to_->is_route_node()) {
        route_nodes.emplace_back(edge.to_);
      }
    }

    return route_nodes;
  }

  std::vector<node*> get_route_nodes() {
    std::vector<node*> route_nodes;

    for (auto& edge : edges_) {
      if (edge.to_->is_route_node()) {
        route_nodes.emplace_back(edge.to_);
      }
    }

    return route_nodes;
  }

  int add_foot_edge(int node_id, edge fe) {
    printf("add_foot_edge\n");
    if (foot_node_ == nullptr) {
      foot_node_ = new node(this, node_id++);
      for (auto& route_node : get_route_nodes()) {
        // check whether it is allowed to transfer at the route-node
        // we do this by checking, whether it has an edge to the station
        for (auto const& edge : route_node->edges_) {
          if (edge.get_destination() == this) {
            // the foot-edge may only be used
            // if a train was used beforewards when
            // trying to use it from a route node
            printf("building after train edge\n");
            route_node->edges_.push_back(
                make_after_train_edge(route_node, foot_node_, 0, true));
            break;
          }
        }
      }
      edges_.emplace_back(make_foot_edge(this, foot_node_));
    }
    fe.from_ = foot_node_;
    foot_node_->edges_.emplace_back(std::move(fe));
    return node_id;
  }

  node* foot_node_;
};

typedef std::unique_ptr<station_node> station_node_ptr;

}  // namespace motis
