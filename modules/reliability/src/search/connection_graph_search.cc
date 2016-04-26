#include "motis/reliability/search/connection_graph_search.h"

#include <limits>
#include <memory>

#include "motis/module/context/motis_call.h"
#include "motis/module/context/motis_parallel_for.h"

#include "motis/core/common/logging.h"

#include "motis/core/journey/journey_util.h"
#include "motis/core/journey/message_to_journeys.h"

#include "motis/core/schedule/schedule.h"
#include "motis/core/schedule/time.h"

#include "motis/protocol/RoutingResponse_generated.h"

#include "motis/reliability/context.h"
#include "motis/reliability/rating/connection_graph_rating.h"
#include "motis/reliability/reliability.h"
#include "motis/reliability/search/cg_optimizer.h"
#include "motis/reliability/search/cg_search_context.h"
#include "motis/reliability/search/connection_graph.h"
#include "motis/reliability/search/connection_graph_builder.h"
#include "motis/reliability/search/connection_graph_search_tools.h"

namespace p = std::placeholders;

namespace motis {
namespace reliability {
namespace search {
namespace connection_graph_search {
namespace detail {

std::vector<unsigned int> insert_stop_states(
    context::conn_graph_context& cg_context) {
  std::vector<unsigned int> new_stops;
  for (auto const& stop : cg_context.cg_->stops_) {
    if (cg_context.stop_states_.find(stop.index_) ==
        cg_context.stop_states_.end()) {
      auto& state = cg_context.stop_states_[stop.index_];
      if (stop.index_ == connection_graph::stop::Index_departure_stop ||
          stop.index_ == connection_graph::stop::Index_arrival_stop) {
        state.state_ = context::conn_graph_context::stop_state::Stop_completed;
      } else {
        new_stops.push_back(stop.index_);
        state.state_ = context::conn_graph_context::stop_state::Stop_idle;
      }
    }
  }
  assert(cg_context.stop_states_.size() == cg_context.cg_->stops_.size());
  return new_stops;
}

void check_stop_states(connection_graph_optimizer const& optimizer,
                       context::conn_graph_context& cg_context,
                       std::vector<unsigned int> const& stop_indices) {
  bool stop_completed = false;
  for (auto const idx : stop_indices) {
    if (idx != connection_graph::stop::Index_departure_stop &&
        idx != connection_graph::stop::Index_arrival_stop) {
      auto& stop_state = cg_context.stop_states_.at(idx);
      if (optimizer.complete(cg_context.cg_->stops_.at(idx), stop_state)) {
        stop_state.state_ =
            context::conn_graph_context::stop_state::Stop_completed;
        stop_completed = true;
      } else {
        stop_state.state_ = context::conn_graph_context::stop_state::Stop_idle;
      }
    }
  }
}

void add_alternative(journey const& j, std::shared_ptr<context> c,
                     context::conn_graph_context& conn_graph,
                     unsigned int const stop_idx) {
  connection_graph_builder::add_alternative_journey(*conn_graph.cg_, stop_idx,
                                                    j);
  std::vector<unsigned int> stop_indices({stop_idx});
  if (conn_graph.stop_states_.size() < conn_graph.cg_->stops_.size()) {
    auto new_indices = insert_stop_states(conn_graph);
    for (auto const idx : new_indices) {
      stop_indices.push_back(idx);
    }
  }

  rating::cg::rate_inserted_alternative(conn_graph, stop_idx,
                                        c->reliability_context_);

  check_stop_states(*c->optimizer_, conn_graph, stop_indices);
}

std::vector<journey> retrieve_base_journeys(
    ReliableRoutingRequest const* request) {
  using routing::RoutingResponse;
  auto routing_response =
      motis_call(
          flatbuffers::request_builder::request_builder(request->request())
              .build_routing_request())
          ->val();
  return message_to_journeys(motis_content(RoutingResponse, routing_response));
}

void init_connection_graph_from_base_journey(context& context,
                                             journey const& base_journey) {
  context.connection_graphs_.emplace_back();
  auto& cg_context = context.connection_graphs_.back();
  cg_context.index_ = context.connection_graphs_.size() - 1;

  connection_graph_builder::add_base_journey(*cg_context.cg_, base_journey);
  auto const stop_indices = insert_stop_states(cg_context);
  rating::cg::rate_inserted_alternative(cg_context, 0,
                                        context.reliability_context_);
  check_stop_states(*context.optimizer_, cg_context, stop_indices);
}

struct request_type {
  unsigned int stop_id_;
  module::msg_ptr request_msg_;
  context::journey_cache_key cache_key_;
};
std::vector<request_type> alternative_requests(context::conn_graph_context& cg,
                                               duration const min_dep_diff) {
  std::vector<request_type> requests;
  for (auto& stop_state : cg.stop_states_) {
    if (stop_state.second.state_ !=
        context::conn_graph_context::stop_state::Stop_idle) {
      continue;
    }

    stop_state.second.state_ =
        context::conn_graph_context::stop_state::Stop_busy;

    auto const req = tools::to_routing_request(
        *cg.cg_, cg.cg_->stops_.at(stop_state.first), min_dep_diff);
    requests.push_back({stop_state.first, req.first, req.second});
  }
  return requests;
}

journey retrieve_alternative(motis::module::msg_ptr const& request) {
  using routing::RoutingResponse;

  std::vector<journey> journeys;
  try {
    auto routing_response = motis_call(request)->val();
    journeys =
        message_to_journeys(motis_content(RoutingResponse, routing_response));
  } catch (...) {
    LOG(logging::warn) << "Failed to retrieve alternative";
  }

  /* note: this method ignores journeys that are
   * corrupt because the state machine in journey.cc
   * can not handle walks at the beginning of journeys
   * (such journeys are found in the on-trip search).
   * This filtering is not necessary as soon as the state
   * machine in journey.cc works correctly. */
  auto const filtered = tools::remove_invalid_journeys(journeys);
  if (filtered.size() != journeys.size() || filtered.empty()) {
    return journey();
  }
  return tools::select_alternative(filtered);
}

void build_cg(context::conn_graph_context& cg, std::shared_ptr<context> c) {
  struct future_return {
    unsigned int stop_id_;
    journey journey_;
    bool is_cached_;
    context::journey_cache_key cache_key_;
  };
  std::vector<ctx::future_ptr<module::ctx_data, future_return>>
      new_alternative_futures;

  // TODO vector with active stops
  // TODO mutex for cache

  do {
    auto const requests =
        alternative_requests(cg, c->optimizer_->min_departure_diff_);

    for (auto const& req : requests) {
      new_alternative_futures.emplace_back(
          module::spawn_job(req, [&](request_type const& req) -> future_return {
            auto const cache_it = c->journey_cache_.find(req.cache_key_);
            bool const is_cached = cache_it != c->journey_cache_.end();
            return future_return{req.stop_id_,
                                 is_cached
                                     ? cache_it->second
                                     : retrieve_alternative(req.request_msg_),
                                 is_cached, req.cache_key_};
          }));
    }

    auto const alternative = new_alternative_futures.front()->val();
    if (!alternative.is_cached_) {
      c->journey_cache_[alternative.cache_key_] = alternative.journey_;
    }

    add_alternative(alternative.journey_, c, cg, alternative.stop_id_);

    new_alternative_futures.erase(new_alternative_futures.begin());
  } while (!new_alternative_futures.empty());
}

}  // namespace detail

std::vector<std::shared_ptr<connection_graph>> search_cgs(
    ReliableRoutingRequest const* request,
    motis::reliability::context const& rel_context,
    std::shared_ptr<connection_graph_optimizer const> optimizer) {
  auto c = std::make_shared<detail::context>(rel_context, optimizer);

  for (auto const& j : detail::retrieve_base_journeys(request)) {
    detail::init_connection_graph_from_base_journey(*c, j);
  }

  using namespace motis::module;
  motis_parallel_for(c->connection_graphs_,
                     std::bind(&detail::build_cg, std::placeholders::_1, c));

  std::vector<std::shared_ptr<connection_graph>> cgs;
  for (auto const& cg_c : c->connection_graphs_) {
    cgs.push_back(cg_c.cg_);
  }
  return cgs;
}

}  // namespace connection_graph_search
}  // namespace search
}  // namespace reliability
}  // namespace motis
