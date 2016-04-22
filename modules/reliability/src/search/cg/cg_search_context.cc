#include "motis/reliability/search/cg_search_context.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "motis/core/schedule/synced_schedule.h"

#include "motis/reliability/context.h"
#include "motis/reliability/distributions/probability_distribution.h"
#include "motis/reliability/reliability.h"
#include "motis/reliability/search/connection_graph.h"
#include "motis/reliability/search/connection_graph_search.h"

namespace motis {
namespace reliability {
namespace search {
namespace connection_graph_search {
namespace detail {

context::context(motis::reliability::reliability& rel,
                 std::shared_ptr<connection_graph_optimizer const> optimizer)
    : reliability_(rel),
      optimizer_(optimizer),
      result_returned_(false),
      synced_sched_(reliability_.synced_sched()),
      reliability_context_(synced_sched_.sched(),
                           reliability_.precomputed_distributions(),
                           reliability_.s_t_distributions()) {}

context::conn_graph_context::conn_graph_context()
    : index_(0),
      cg_(std::make_shared<connection_graph>()),
      cg_state_(CG_in_progress) {}

bool context::journey_cache_key::operator<(
    journey_cache_key const& right) const {
  return from_eva_ < right.from_eva_ ||
         (from_eva_ == right.from_eva_ && begin_time_ < right.begin_time_) ||
         (from_eva_ == right.from_eva_ && begin_time_ == right.begin_time_ &&
          end_time_ < right.end_time_);
}

}  // namespace detail
}  // namespace connection_graph_search
}  // namespace search
}  // namespace reliability
}  // namespace motis
