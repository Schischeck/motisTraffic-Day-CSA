#pragma once

#include <algorithm>
#include <map>
#include <string>

#include "motis/core/schedule/connection.h"
#include "motis/core/schedule/schedule.h"
#include "motis/core/schedule/time.h"

#include "motis/reliability/distributions/probability_distribution.h"
#include "motis/reliability/realtime/time_util.h"

namespace motis {
namespace reliability {
namespace distributions_container {

struct container {
  struct key {
    uint32_t train_id_;
    std::string category_;
    std::string line_identifier_;
    uint32_t station_index_;
    time_util::event_type type_;
    motis::time scheduled_event_time_;

    key()
        : train_id_(0),
          category_(""),
          line_identifier_(""),
          station_index_(0),
          type_(time_util::departure),
          scheduled_event_time_(0) {}
    key(uint32_t train_id, std::string category, std::string line_identifier,
        uint32_t station_index, time_util::event_type type,
        motis::time scheduled_event_time)
        : train_id_(train_id),
          category_(to_lower(category)),
          line_identifier_(line_identifier),
          station_index_(station_index),
          type_(type),
          scheduled_event_time_(scheduled_event_time) {}

    bool operator<(key const& o) const {
      return std::tie(train_id_, category_, line_identifier_, station_index_,
                      type_, scheduled_event_time_) <
             std::tie(o.train_id_, o.category_, o.line_identifier_,
                      o.station_index_, o.type_, o.scheduled_event_time_);
    }

    bool operator==(key const& o) const {
      return std::tie(train_id_, category_, line_identifier_, station_index_,
                      type_, scheduled_event_time_) ==
             std::tie(o.train_id_, o.category_, o.line_identifier_,
                      o.station_index_, o.type_, o.scheduled_event_time_);
    }

  private:
    static std::string to_lower(std::string str) {
      std::transform(str.begin(), str.end(), str.begin(), ::tolower);
      return str;
    };
  };

  struct node {
    key key_;
    probability_distribution pd_;
    std::vector<node*> successors_;
    std::vector<node*> predecessors_;
  };

  container() : invalid_node_(std::make_shared<node>()) {}

  virtual ~container() {}

  virtual probability_distribution const& get_distribution(key const& k) const {
    auto it = distributions_nodes_.find(k);
    if (it != distributions_nodes_.end()) {
      return it->second->pd_;
    }
    return invalid_node_->pd_;
  }

  virtual node const& get_node(key const& k) const {
    auto it = distributions_nodes_.find(k);
    if (it != distributions_nodes_.end()) {
      return *it->second;
    }
    return *invalid_node_;
  }

  virtual node& get_node_non_const(key const& k) {
    auto& n = distributions_nodes_[k];
    if (!n) {
      n = std::make_shared<node>();
      n->key_ = k;
    }
    return *n;
  }

  virtual bool contains_distribution(key const& k) const {
    auto it = distributions_nodes_.find(k);
    if (it != distributions_nodes_.end()) {
      return true;
    }
    return false;
  }

private:
  std::map<key, std::shared_ptr<node> > distributions_nodes_;
  std::shared_ptr<node> invalid_node_;
};

inline std::ostream& operator<<(std::ostream& out, container::key const& k) {
  out << "key: tr=" << k.train_id_ << " cat='" << k.category_ << "'"
      << " line='" << k.line_identifier_ << "'"
      << " st=" << k.station_index_
      << " tp=" << (k.type_ == time_util::departure ? "d" : "a")
      << " t=" << format_time(k.scheduled_event_time_) << std::endl;
  return out;
}

struct single_distribution_container : container {
  single_distribution_container(probability_distribution const& distribution)
      : distribution_(distribution) {}
  probability_distribution const& get_distribution(key const&) const override {
    return distribution_;
  };

private:
  probability_distribution const& distribution_;
};

inline container::key to_container_key(light_connection const& lc,
                                       unsigned int const station_idx,
                                       time_util::event_type const type,
                                       motis::time const scheduled_event_time,
                                       schedule const& sched) {
  auto const& conn_info = *lc._full_con->con_info;
  return container::key(
      conn_info.train_nr, sched.categories.at(conn_info.family)->name,
      conn_info.line_identifier, station_idx, type, scheduled_event_time);
}

inline container::key to_container_key(node const& route_node,
                                       light_connection const& lc,
                                       time_util::event_type const type,
                                       schedule const& sched) {
  return to_container_key(
      lc, route_node.get_station()->_id, type,
      time_util::get_scheduled_event_time(route_node, lc, type, sched), sched);
};

}  // namespace distributions_container
}  // namespace reliability
}  // namespace motis
