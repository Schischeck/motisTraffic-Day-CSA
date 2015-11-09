#pragma once

#include <map>
#include <memory>

#include "motis/core/schedule/synced_schedule.h"

#include "motis/module/sid.h"

#include "motis/reliability/context.h"
#include "motis/reliability/probability_distribution.h"
#include "motis/reliability/search/cg_search_callback.h"

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
      unsigned short num_failed_requests_ = 0;
      probability_distribution uncovered_arrival_distribution_;
    };
    std::map<unsigned int, stop_state> stop_states_;
  };

  context(motis::reliability::reliability& rel, motis::module::sid session,
          /*motis::reliability::search::connection_graph_search::*/ callback cb,
          connection_graph_optimizer const& optimizer);

  std::vector<conn_graph_context> connection_graphs_;
  motis::reliability::reliability& reliability_;
  motis::module::sid session_;
  callback result_callback_;
  connection_graph_optimizer const& optimizer_;
  bool result_returned_;

  struct journey_cache_key {
    journey_cache_key(std::string const& from_eva, time_t const& begin_time,
                      time_t const& end_time)
        : from_eva_(from_eva), begin_time_(begin_time), end_time_(end_time) {}
    bool operator<(journey_cache_key const& right) const;
    std::string const from_eva_;
    time_t const begin_time_;
    time_t const end_time_;
  };
  std::map<journey_cache_key, journey> journey_cache;

  synced_schedule<RO> synced_sched_;
  motis::reliability::context reliability_context_;
};

}  // namespace detail
}  // namespace connection_graph_search
}  // namespace search
}  // namespace reliability
}  // namespace motis
