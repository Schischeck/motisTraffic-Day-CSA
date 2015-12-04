#pragma once

#include <map>
#include <string>

#include "motis/core/schedule/connection.h"
#include "motis/core/schedule/time.h"

#include "motis/reliability/distributions/probability_distribution.h"

namespace motis {
namespace reliability {
namespace distributions_container {

struct container {
  struct key {
    uint32_t train_id_;
    uint32_t family_;
    std::string line_identifier_;
    uint32_t station_index_;
    enum event_type { departure, arrival } type_;
    motis::time scheduled_event_time_;

    bool operator<(const key& o) const {
      return std::tie(train_id_, family_, line_identifier_, station_index_,
                      type_, scheduled_event_time_) <
             std::tie(o.train_id_, o.family_, o.line_identifier_,
                      o.station_index_, o.type_, o.scheduled_event_time_);
    }
  };

  virtual ~container() {}

  virtual probability_distribution const& get_distribution(key const& k) const {
    auto it = distributions_.find(k);
    if (it != distributions_.end()) {
      return it->second;
    }
    return invalid_distribution_;
  }

  virtual probability_distribution& get_distribution_non_const(key const& k) {
    return distributions_[k];
  }

  virtual bool contains_distribution(key const& k) const {
    auto it = distributions_.find(k);
    if (it != distributions_.end()) {
      return true;
    }
    return false;
  }

private:
  std::map<key, probability_distribution> distributions_;
  probability_distribution invalid_distribution_;
};

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
                                       container::key::event_type type,
                                       motis::time scheduled_event_time) {
  auto const& conn_info = *lc._full_con->con_info;
  return {conn_info.train_nr,
          conn_info.family,
          conn_info.line_identifier,
          station_idx,
          type,
          scheduled_event_time};
};

}  // namespace distributions_container
}  // namespace reliability
}  // namespace motis
