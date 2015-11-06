#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "motis/core/schedule/synced_schedule.h"

#include "motis/reliability/context.h"
#include "motis/reliability/probability_distribution.h"
#include "motis/reliability/reliability.h"
#include "motis/reliability/search/connection_graph.h"
#include "motis/reliability/search/connection_graph_search.h"

namespace motis {
namespace reliability {
struct reliability;
namespace search {
namespace connection_graph_search {
struct connection_graph_optimizer;
namespace detail {

struct context {
  struct conn_graph_context {
    conn_graph_context()
        : index_(0),
          cg_(std::make_shared<connection_graph>()),
          cg_state_(CG_in_progress) {}
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
          callback cb, connection_graph_optimizer const& optimizer)
      : reliability_(rel),
        session_(session),
        result_callback_(cb),
        optimizer_(optimizer),
        result_returned_(false),
        synced_sched_(reliability_.synced_sched()),
        reliability_context_(synced_sched_.sched(),
                             reliability_.precomputed_distributions(),
                             reliability_.s_t_distributions()) {}

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
    bool operator<(journey_cache_key const& right) const {
      return from_eva_ < right.from_eva_ || (from_eva_ == right.from_eva_ &&
                                             begin_time_ < right.begin_time_) ||
             (from_eva_ == right.from_eva_ &&
              begin_time_ == right.begin_time_ && end_time_ < right.end_time_);
    }
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
