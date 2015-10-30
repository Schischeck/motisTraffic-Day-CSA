#include <limits>
#include <memory>

#include "motis/reliability/search/connection_graph_search.h"

#include "motis/core/schedule/schedule.h"

#include "motis/core/common/journey_builder.h"

#include "motis/protocol/RoutingResponse_generated.h"

#include "motis/reliability/reliability.h"
#include "motis/reliability/search/connection_graph.h"
#include "motis/reliability/search/connection_graph_builder.h"
#include "motis/reliability/tools/flatbuffers_tools.h"

namespace p = std::placeholders;

namespace motis {
namespace reliability {
namespace search {
namespace connection_graph_search {
namespace detail {
struct context {
  struct conn_graph_context {
    conn_graph_context()
        : cg_(std::make_shared<connection_graph>()),
          cg_state_(CG_in_progress) {}
    std::shared_ptr<connection_graph> cg_;
    enum cg_state { CG_in_progress, CG_completed, CG_failed } cg_state_;
    enum stop_state { Stop_idle, Stop_busy, Stop_completed };
    std::map<unsigned int, stop_state> stop_states_;
  };
  context(motis::reliability::reliability& rel, schedule const& sched,
          motis::module::sid session, callback cb, complete_func f)
      : reliability_(rel),
        schedule_(sched),
        session_(session),
        result_callback_(cb),
        f_(f),
        result_returned_(false) {}

  std::vector<conn_graph_context> connection_graphs_;
  motis::reliability::reliability& reliability_;
  schedule const& schedule_;
  motis::module::sid session_;
  callback result_callback_;
  complete_func f_;
  bool result_returned_;
};
enum build_state { Base_failed, CG_completed, CG_failed };
void build_result(build_state, std::shared_ptr<context>);
void handle_base_response(motis::module::msg_ptr, boost::system::error_code,
                          std::shared_ptr<context>);
void handle_alternative_response(motis::module::msg_ptr,
                                 boost::system::error_code,
                                 std::shared_ptr<context>,
                                 context::conn_graph_context&,
                                 unsigned int const stop_idx);

bool complete(context::conn_graph_context const& cg) {
  return std::find_if(cg.cg_->stops.begin(), cg.cg_->stops.end(),
                      [&](connection_graph::stop const& stop) {
                        auto const it = cg.stop_states_.find(stop.index);
                        return it == cg.stop_states_.end() ||
                               it->second != context::conn_graph_context::
                                                 stop_state::Stop_completed;
                      }) == cg.cg_->stops.end();
}
bool complete(context const& c) {
  return std::find_if(c.connection_graphs_.begin(), c.connection_graphs_.end(),
                      [&](context::conn_graph_context const& cg) {
                        return cg.cg_state_ ==
                               context::conn_graph_context::CG_in_progress;
                      }) == c.connection_graphs_.end();
}

inline journey const& latest_departing_alternative(
    connection_graph const& conn_graph, connection_graph::stop const& stop) {
  return std::find_if(conn_graph.journeys.rbegin(), conn_graph.journeys.rend(),
                      [stop](connection_graph::journey_info const& j) {
                        return stop.index == j.from_index;
                      })->j;
}

module::msg_ptr to_routing_request(connection_graph& conn_graph,
                                   connection_graph::stop const& stop) {
  auto const& last_alternative = latest_departing_alternative(conn_graph, stop);
  time const time_begin = last_alternative.stops.back().arrival.timestamp;
  time const time_end = time_begin + (15 * 60);
  auto const& arrival_stop =
      conn_graph.stops.at(connection_graph::stop::Index_arrival_stop);
  return flatbuffers_tools::to_routing_request(
      stop.name, stop.eva_no, arrival_stop.name, arrival_stop.eva_no,
      time_begin, time_end);
}

void build_cg(std::shared_ptr<context> c,
              context::conn_graph_context& conn_graph) {
  if (conn_graph.cg_state_ == context::conn_graph_context::CG_in_progress) {
    for (auto& stop : conn_graph.cg_->stops) {
      auto it = conn_graph.stop_states_.find(stop.index);
      if (it == conn_graph.stop_states_.end() ||
          it->second == context::conn_graph_context::stop_state::Stop_idle)
        return c->reliability_.send_message(
            to_routing_request(*conn_graph.cg_, stop), c->session_,
            std::bind(&handle_alternative_response, p::_1, p::_2, c,
                      std::ref(conn_graph), stop.index));
    }
  } else {
    return build_result(CG_completed, c);
  }
  /* unexpected case */
  return build_result(CG_failed, c);
}

void handle_base_response(motis::module::msg_ptr msg,
                          boost::system::error_code e,
                          std::shared_ptr<context> context) {
  if (e) {
    return build_result(Base_failed, context);
  }
  auto journeys = journey_builder::to_journeys(
      msg->content<routing::RoutingResponse const*>(),
      context->schedule_.categories);
  if (journeys.size() == 0) {
    return build_result(Base_failed, context);
  }

  for (auto const& j : journeys) {
    context->connection_graphs_.emplace_back();
    auto& conn_graph = *context->connection_graphs_.back().cg_;
    connection_graph_builder::add_base_journey(conn_graph, j);
    /* todo: rate cg */
  }

  for (auto conn_graph : context->connection_graphs_) {
    build_cg(context, conn_graph);
  }
}

inline journey const& select_alternative(connection_graph const& conn_graph,
                                         std::vector<journey> const& journeys) {
  /* earliest arrival */
  return std::ref(*std::min_element(journeys.begin(), journeys.end(),
                                    [](journey const& a, journey const& b) {
                                      return a.stops.back().arrival.timestamp <
                                             b.stops.back().arrival.timestamp;
                                    }));
}

void handle_alternative_response(motis::module::msg_ptr msg,
                                 boost::system::error_code e,
                                 std::shared_ptr<context> c,
                                 context::conn_graph_context& conn_graph,
                                 unsigned int const stop_idx) {
  if (e) {
    return build_result(CG_failed, c);
  }
  auto journeys = journey_builder::to_journeys(
      msg->content<routing::RoutingResponse const*>(), c->schedule_.categories);
  auto const& j = select_alternative(*conn_graph.cg_.get(), journeys);
  connection_graph_builder::add_alternative_journey(
      std::ref(*conn_graph.cg_.get()), stop_idx, j);
  /* todo: rate cg */
  conn_graph.stop_states_[stop_idx] =
      context::conn_graph_context::stop_state::Stop_idle;
  if (c->f_(conn_graph.cg_->stops.at(stop_idx), *conn_graph.cg_)) {
    conn_graph.stop_states_[stop_idx] =
        context::conn_graph_context::stop_state::Stop_completed;
    if (complete(conn_graph)) {
      conn_graph.cg_state_ =
          context::conn_graph_context::cg_state::CG_completed;
    }
  }

  build_cg(c, conn_graph);
}

void build_result(build_state state, std::shared_ptr<context> c) {
  if (c->result_returned_) {
    return;
  } else if (state == Base_failed) {
    c->result_returned_ = true;
    return c->result_callback_({});
  } else if (complete(*c)) {
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
                motis::reliability::reliability& rel, schedule const& sched,
                motis::module::sid session, complete_func f, callback cb) {
  rel.send_message(
      flatbuffers_tools::to_flatbuffers_message(request->request()), session,
      std::bind(&detail::handle_base_response, p::_1, p::_2,
                std::make_shared<detail::context>(rel, sched, session, cb, f)));
}

}  // namespace connection_graph_search
}  // namespace search
}  // namespace reliability
}  // namespace motis
