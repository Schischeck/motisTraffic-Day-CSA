#include "motis/reliability/search/connection_graph_search.h"

#include <limits>
#include <memory>

#include "motis/core/schedule/schedule.h"
#include "motis/core/schedule/time.h"

#include "motis/core/common/journey_builder.h"

#include "motis/protocol/RoutingResponse_generated.h"

#include "motis/reliability/context.h"
#include "motis/reliability/reliability.h"
#include "motis/reliability/rating/connection_graph_rating.h"
#include "motis/reliability/search/cg_search_context.h"
#include "motis/reliability/search/connection_graph.h"
#include "motis/reliability/search/connection_graph_builder.h"
#include "motis/reliability/search/connection_graph_search_tools.h"
#include "motis/reliability/search/cg_optimizer.h"

namespace p = std::placeholders;

namespace motis {
namespace reliability {
namespace search {
namespace connection_graph_search {
namespace detail {
void build_result(context::conn_graph_context::cg_state,
                  std::shared_ptr<context>);
void handle_base_response(motis::module::msg_ptr, boost::system::error_code,
                          std::shared_ptr<context>);
void handle_alternative_response(motis::module::msg_ptr,
                                 boost::system::error_code,
                                 std::shared_ptr<context>,
                                 unsigned int const conn_graph_idx,
                                 unsigned int const stop_idx,
                                 context::journey_cache_key const);
void add_alternative(journey const&, std::shared_ptr<context>,
                     context::conn_graph_context&, unsigned int const stop_idx);

void search_for_alternative(std::shared_ptr<context> c,
                            context::conn_graph_context& conn_graph,
                            connection_graph::stop& stop) {
  auto request = tools::to_routing_request(*conn_graph.cg_, stop,
                                           c->optimizer_->min_departure_diff_);

  auto cache_it = c->journey_cache.find(request.second);
  if (cache_it != c->journey_cache.end()) {
    /* todo: do not copy cached journeys!
     * store and output (json) each journey once */
    return add_alternative(cache_it->second, c, conn_graph, stop.index_);
  }

  return c->reliability_.send_message(
      request.first, c->session_,
      std::bind(&handle_alternative_response, p::_1, p::_2, c,
                conn_graph.index_, stop.index_, request.second));
}

void build_cg(std::shared_ptr<context> c,
              context::conn_graph_context& conn_graph) {
  if (conn_graph.cg_state_ == context::conn_graph_context::CG_in_progress) {
    for (auto& stop : conn_graph.cg_->stops_) {
      auto it = conn_graph.stop_states_.find(stop.index_);
      assert(it != conn_graph.stop_states_.end());
      if (it->second.state_ ==
          context::conn_graph_context::stop_state::Stop_idle) {
        it->second.state_ = context::conn_graph_context::stop_state::Stop_busy;
        /* note: If a cached journey is used,
         * build_cg is called synchronously in search_for_alternative*/
        search_for_alternative(c, conn_graph, stop);
      }
    }
    return; /* wait for search results */
  } else {
    return build_result(context::conn_graph_context::CG_completed, c);
  }
  std::cout << "\nbuild_cg: unexpected case" << std::endl;
  /* unexpected case */
  return build_result(context::conn_graph_context::CG_failed, c);
}

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
  if (stop_completed && tools::complete(cg_context)) {
    cg_context.cg_state_ = context::conn_graph_context::cg_state::CG_completed;
  }
}

void init_connection_graph_from_base_journey(context& context,
                                             journey const& base_journey) {
  context.connection_graphs_.emplace_back();
  auto& cg_context = context.connection_graphs_.back();
  cg_context.index_ = context.connection_graphs_.size() - 1;
  cg_context.cg_state_ = context::conn_graph_context::CG_in_progress;

  connection_graph_builder::add_base_journey(*cg_context.cg_, base_journey);
  auto const stop_indices = insert_stop_states(cg_context);
  rating::cg::rate_inserted_alternative(cg_context, 0,
                                        context.reliability_context_);
  check_stop_states(*context.optimizer_, cg_context, stop_indices);

  if (cg_context.cg_->journeys_.size() == 1) {
    cg_context.cg_state_ = context::conn_graph_context::CG_completed;
  }
}

void handle_base_response(motis::module::msg_ptr msg,
                          boost::system::error_code e,
                          std::shared_ptr<context> context) {
  if (e) {
    return build_result(context::conn_graph_context::CG_base_failed, context);
  }
  auto journeys = journey_builder::to_journeys(
      msg->content<routing::RoutingResponse const*>());
  if (journeys.empty()) {
    return build_result(context::conn_graph_context::CG_base_failed, context);
  }

  for (auto const& j : journeys) {
    init_connection_graph_from_base_journey(*context, j);
  }

  for (auto conn_graph : context->connection_graphs_) {
    build_cg(context, conn_graph);
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

  build_cg(c, conn_graph);
}

void handle_alternative_response(motis::module::msg_ptr msg,
                                 boost::system::error_code e,
                                 std::shared_ptr<context> c,
                                 unsigned int const conn_graph_idx,
                                 unsigned int const stop_idx,
                                 context::journey_cache_key const cache_key) {
  if (e) {
    return build_result(context::conn_graph_context::CG_failed, c);
  }
  auto& cg_context = c->connection_graphs_.at(conn_graph_idx);
  assert(cg_context.stop_states_.size() == cg_context.cg_->stops_.size());
  auto& stop_state = cg_context.stop_states_.at(stop_idx);

  auto journeys = journey_builder::to_journeys(
      msg->content<routing::RoutingResponse const*>());
  /* note: this method ignores journeys that are
   * corrupt because the state machine in journey.cc
   * can not handle walks at the beginning of journeys
   * (such journeys are found in the on-trip search).
   * This filtering is not necessary as soon as the state
   * machine in journey.cc works correctly. */
  auto filtered = tools::remove_invalid_journeys(journeys);

  if (filtered.empty()) {
    stop_state.state_ = context::conn_graph_context::stop_state::Stop_completed;
    if (tools::complete(cg_context)) {
      cg_context.cg_state_ =
          context::conn_graph_context::cg_state::CG_completed;
    }
    return build_cg(c, cg_context);
  }

  auto const& j = tools::select_alternative(filtered);
  assert(c->journey_cache.find(cache_key) == c->journey_cache.end());
  c->journey_cache[cache_key] = j;
  add_alternative(j, c, cg_context, stop_idx);
}

void build_result(context::conn_graph_context::cg_state state,
                  std::shared_ptr<context> c) {
  if (c->result_returned_) {
    return;
  } else if (state == context::conn_graph_context::CG_base_failed ||
             state == context::conn_graph_context::CG_failed) {
    c->result_returned_ = true;
    return c->result_callback_({});
  } else if (tools::complete(*c)) {
    c->result_returned_ = true;
    std::vector<std::shared_ptr<connection_graph> > cgs;
    for (auto const& cg_c : c->connection_graphs_) {
      if (cg_c.cg_state_ == context::conn_graph_context::CG_completed) {
        cgs.push_back(cg_c.cg_);
      }
    }
    return c->result_callback_(cgs);
  }
}

}  // namespace detail

void search_cgs(ReliableRoutingRequest const* request,
                motis::reliability::reliability& rel,
                motis::module::sid session,
                std::shared_ptr<connection_graph_optimizer const> optimizer,
                callback cb) {
  rel.send_message(
      flatbuffers_tools::to_flatbuffers_message(request->request()), session,
      std::bind(
          &detail::handle_base_response, p::_1, p::_2,
          std::make_shared<detail::context>(rel, session, cb, optimizer)));
}

}  // namespace connection_graph_search
}  // namespace search
}  // namespace reliability
}  // namespace motis
