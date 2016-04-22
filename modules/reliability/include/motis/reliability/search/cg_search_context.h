#pragma once

#include <map>
#include <memory>

#include "motis/module/module.h"

#include "motis/reliability/context.h"
#include "motis/reliability/distributions/probability_distribution.h"

namespace motis {
struct journey;
namespace reliability {
struct reliability;
namespace search {
struct connection_graph;
namespace connection_graph_search {
struct connection_graph_optimizer;
namespace detail {

struct context {
  struct conn_graph_context {
    conn_graph_context();
    unsigned int index_;
    std::shared_ptr<connection_graph> cg_;
    enum cg_state {
      CG_in_progress,
      CG_completed,
      CG_base_failed,
      CG_failed
    } cg_state_;

    struct stop_state {
      enum state {
        Stop_idle,
        Stop_busy,
        Stop_completed,
        Stop_Aborted
      } state_ = Stop_idle;
      probability_distribution uncovered_arrival_distribution_;
    };
    std::map<unsigned int, stop_state> stop_states_;
  };

  context(motis::reliability::reliability&,
          std::shared_ptr<connection_graph_optimizer const>);

  std::vector<conn_graph_context> connection_graphs_;
  motis::reliability::reliability& reliability_;
  std::shared_ptr<connection_graph_optimizer const> optimizer_;
  bool result_returned_;

  struct journey_cache_key {
    journey_cache_key(std::string const& from_eva, time_t const& ontrip_time)
        : from_eva_(from_eva), ontrip_time_(ontrip_time) {}
    bool operator<(journey_cache_key const& right) const;
    std::string const from_eva_;
    time_t const ontrip_time_;
  };
  std::map<journey_cache_key, journey> journey_cache_;

  synced_schedule<RO> synced_sched_;
  motis::reliability::context reliability_context_;
};

}  // namespace detail
}  // namespace connection_graph_search
}  // namespace search
}  // namespace reliability
}  // namespace motis
