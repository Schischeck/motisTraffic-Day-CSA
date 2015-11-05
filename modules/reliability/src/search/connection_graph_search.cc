#include "motis/reliability/search/connection_graph_search.h"

#include <limits>
#include <memory>
#include <fstream>

#include "motis/core/schedule/schedule.h"
#include "motis/core/schedule/time.h"

#include "motis/core/common/journey_builder.h"

#include "motis/protocol/RoutingResponse_generated.h"

#include "motis/reliability/reliability.h"
#include "motis/reliability/rating/connection_graph_rating.h"
#include "motis/reliability/search/cg_search_context.h"
#include "motis/reliability/search/connection_graph.h"
#include "motis/reliability/search/connection_graph_builder.h"
#include "motis/reliability/search/connection_graph_search_tools.h"
#include "motis/reliability/search/simple_connection_graph_optimizer.h"

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
                     context::conn_graph_context&,
                     context::conn_graph_context::stop_state&,
                     unsigned int const stop_idx);

void search_for_alternative(
    std::shared_ptr<context> c, context::conn_graph_context& conn_graph,
    connection_graph::stop& stop,
    context::conn_graph_context::stop_state& stop_state) {
  auto request = tools::to_routing_request(
      *conn_graph.cg_, stop, c->optimizer_.min_departure_diff_,
      c->optimizer_.interval_width_, stop_state.num_failed_requests);
  auto cache_it = c->journey_cache.find(request.second);
  if (cache_it != c->journey_cache.end()) {
    /* todo: do not copy cached journeys!
     * store and output (json) each journey once */
    return add_alternative(cache_it->second, c, conn_graph, stop_state,
                           stop.index_);
  }
  return c->reliability_.send_message(
      request.first, c->session_,
      std::bind(&handle_alternative_response, p::_1, p::_2, c,
                conn_graph.index_, stop.index_, request.second));
}

void build_cg(std::shared_ptr<context> c,
              context::conn_graph_context& conn_graph) {
  /* todo remove debug output */
  std::ofstream os("CGs.txt");
  for (auto const& cg : c->connection_graphs_) {
    os << "\n\nCG " << cg.index_ << " state " << cg.cg_state_ << std::endl;
    for (auto it = cg.stop_states_.begin(); it != cg.stop_states_.end(); ++it) {
      os << "Stop " << it->first << " state s" << it->second.state_ << " f"
         << it->second.num_failed_requests << " #"
         << cg.cg_->stops_[it->first].alternative_infos_.size() << " "
         << cg.cg_->station_info(it->first).first << std::endl;
    }
  }

  if (conn_graph.cg_state_ == context::conn_graph_context::CG_in_progress) {
    for (auto& stop : conn_graph.cg_->stops_) {
      auto it = conn_graph.stop_states_.find(stop.index_);
      if (it == conn_graph.stop_states_.end() ||
          it->second.state_ ==
              context::conn_graph_context::stop_state::Stop_idle) {
        it->second.state_ = context::conn_graph_context::stop_state::Stop_busy;
        return search_for_alternative(c, conn_graph, stop, it->second);
      }
    }
  } else {
    return build_result(context::conn_graph_context::CG_completed, c);
  }
  /* unexpected case */
  return build_result(context::conn_graph_context::CG_failed, c);
}

void insert_stop_states(context const& c,
                        context::conn_graph_context& cg_context) {
  for (auto const& stop : cg_context.cg_->stops_) {
    if (cg_context.stop_states_.find(stop.index_) ==
        cg_context.stop_states_.end()) {
      auto& state = cg_context.stop_states_[stop.index_];
      state.num_failed_requests = 0;
      if (stop.index_ == connection_graph::stop::Index_departure_stop ||
          stop.index_ == connection_graph::stop::Index_arrival_stop ||
          c.optimizer_.complete(stop, *cg_context.cg_)) {
        state.state_ = context::conn_graph_context::stop_state::Stop_completed;
      } else {
        state.state_ = context::conn_graph_context::stop_state::Stop_idle;
      }
    }
  }
  assert(cg_context.stop_states_.size() == cg_context.cg_->stops_.size());
}

void handle_base_response(motis::module::msg_ptr msg,
                          boost::system::error_code e,
                          std::shared_ptr<context> context) {
  if (e) {
    return build_result(context::conn_graph_context::CG_base_failed, context);
  }

  auto const lock = context->reliability_.synced_sched();
  schedule const& schedule = lock.sched();
  auto journeys = journey_builder::to_journeys(
      msg->content<routing::RoutingResponse const*>(), schedule.categories);
  if (journeys.empty()) {
    return build_result(context::conn_graph_context::CG_base_failed, context);
  }

  for (auto const& j : journeys) {
    context->connection_graphs_.emplace_back();
    auto& cg_context = context->connection_graphs_.back();
    cg_context.index_ = context->connection_graphs_.size() - 1;
    auto& conn_graph = *cg_context.cg_;
    connection_graph_builder::add_base_journey(conn_graph, j);
    rating::cg::rate(conn_graph, 0,
                     context->reliability_.synced_sched().sched(),
                     context->reliability_.precomputed_distributions(),
                     context->reliability_.s_t_distributions());

    insert_stop_states(*context, cg_context);

    if (conn_graph.journeys_.size() == 1 || tools::complete(cg_context)) {
      cg_context.cg_state_ = context::conn_graph_context::CG_completed;
    } else {
      cg_context.cg_state_ = context::conn_graph_context::CG_in_progress;
    }
  }

  for (auto conn_graph : context->connection_graphs_) {
    build_cg(context, conn_graph);
  }
}

void check_stop_state(context::conn_graph_context::stop_state& stop_state,
                      std::shared_ptr<context> c,
                      connection_graph::stop const& stop,
                      context::conn_graph_context& conn_graph) {
  stop_state.num_failed_requests = 0;
  if (conn_graph.stop_states_.size() < conn_graph.cg_->stops_.size()) {
    insert_stop_states(*c, conn_graph);
  }
  if (c->optimizer_.complete(stop, *conn_graph.cg_)) {
    stop_state.state_ = context::conn_graph_context::stop_state::Stop_completed;
    if (tools::complete(conn_graph)) {
      conn_graph.cg_state_ =
          context::conn_graph_context::cg_state::CG_completed;
    }
  } else {
    stop_state.state_ = context::conn_graph_context::stop_state::Stop_idle;
  }
}

void add_alternative(journey const& j, std::shared_ptr<context> c,
                     context::conn_graph_context& conn_graph,
                     context::conn_graph_context::stop_state& stop_state,
                     unsigned int const stop_idx) {
  auto& cg = *conn_graph.cg_;
  connection_graph_builder::add_alternative_journey(std::ref(cg), stop_idx, j);
  rating::cg::rate(cg, stop_idx, c->reliability_.synced_sched().sched(),
                   c->reliability_.precomputed_distributions(),
                   c->reliability_.s_t_distributions());
  check_stop_state(stop_state, c, conn_graph.cg_->stops_.at(stop_idx),
                   conn_graph);
  build_cg(c, conn_graph);
}

std::ofstream os_j("journeys.txt");

void handle_alternative_response(motis::module::msg_ptr msg,
                                 boost::system::error_code e,
                                 std::shared_ptr<context> c,
                                 unsigned int const conn_graph_idx,
                                 unsigned int const stop_idx,
                                 context::journey_cache_key const cache_key) {
  if (e) {
    return build_result(context::conn_graph_context::CG_failed, c);
  }

  auto& conn_graph = c->connection_graphs_.at(conn_graph_idx);
  assert(conn_graph.stop_states_.size() == conn_graph.cg_->stops_.size());
  auto& stop_state = conn_graph.stop_states_.at(stop_idx);

  auto const lock = c->reliability_.synced_sched();
  schedule const& schedule = lock.sched();
  auto journeys = journey_builder::to_journeys(
      msg->content<routing::RoutingResponse const*>(), schedule.categories);

  if (journeys.empty()) {
    ++stop_state.num_failed_requests;
    if (stop_state.num_failed_requests > 5) { /* todo */
      stop_state.state_ =
          context::conn_graph_context::stop_state::Stop_completed;
    }
    return build_cg(c, conn_graph);
  }

  auto const& j = tools::select_alternative(*conn_graph.cg_.get(), journeys);
  assert(c->journey_cache.find(cache_key) == c->journey_cache.end());
  c->journey_cache[cache_key] = j;

  tools::output(j, c->reliability_.synced_sched().sched(), os_j);
  os_j << std::endl;
  add_alternative(j, c, conn_graph, stop_state, stop_idx);
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
                connection_graph_optimizer const& optimizer, callback cb) {
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
