#pragma once

#include <map>
#include <memory>
#include <mutex>

#include "motis/module/module.h"

#include "motis/reliability/context.h"
#include "motis/reliability/distributions/probability_distribution.h"

namespace motis {
struct journey;
namespace reliability {
namespace search {
struct connection_graph;
namespace connection_graph_search {
struct connection_graph_optimizer;
namespace detail {

struct context {
  struct conn_graph_context {
    conn_graph_context()
        : index_(0), cg_(std::make_shared<connection_graph>()) {}
    unsigned int index_;
    std::shared_ptr<connection_graph> cg_;

    struct stop_state {
      probability_distribution uncovered_arrival_distribution_;
    };

    /* positions analogous to connection_graph::stops_ */
    std::vector<stop_state> stop_states_;
  };

  context(motis::reliability::context const& rel_context,
          std::shared_ptr<connection_graph_optimizer const> optimizer)
      : reliability_context_(rel_context), optimizer_(optimizer) {}

  motis::reliability::context reliability_context_;

  std::vector<conn_graph_context> connection_graphs_;
  std::shared_ptr<connection_graph_optimizer const> optimizer_;

  struct journey_cache_key {
    journey_cache_key() = default;
    journey_cache_key(std::string const& from_eva, time_t const& ontrip_time)
        : from_eva_(from_eva), ontrip_time_(ontrip_time) {}
    bool operator<(journey_cache_key const& right) const {
      return from_eva_ < right.from_eva_ || (from_eva_ == right.from_eva_ &&
                                             ontrip_time_ < right.ontrip_time_);
    }
    std::string from_eva_;
    time_t ontrip_time_;
  };
  std::pair<std::mutex, std::map<journey_cache_key, journey>> journey_cache_;
};

}  // namespace detail
}  // namespace connection_graph_search
}  // namespace search
}  // namespace reliability
}  // namespace motis
