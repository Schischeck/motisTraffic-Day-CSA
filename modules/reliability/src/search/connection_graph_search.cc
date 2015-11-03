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
        : index_(0),
          cg_(std::make_shared<connection_graph>()),
          cg_state_(CG_in_progress) {}
    unsigned int index_;
    std::shared_ptr<connection_graph> cg_;
    enum cg_state { CG_in_progress, CG_completed, CG_failed } cg_state_;

    struct stop_state {
      enum state { Stop_idle, Stop_busy, Stop_completed } state_ = Stop_idle;
      unsigned short num_failed_requests = 0;
    };
    std::map<unsigned int, stop_state> stop_states_;
  };
  context(motis::reliability::reliability& rel, motis::module::sid session,
          callback cb, complete_func f)
      : reliability_(rel),
        session_(session),
        result_callback_(cb),
        f_(f),
        result_returned_(false) {}

  std::vector<conn_graph_context> connection_graphs_;
  motis::reliability::reliability& reliability_;
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
                                 unsigned int const conn_graph_idx,
                                 unsigned int const stop_idx);

bool complete(context::conn_graph_context const& cg) {
  return std::find_if(cg.cg_->stops_.begin(), cg.cg_->stops_.end(),
                      [&](connection_graph::stop const& stop) {
                        auto const it = cg.stop_states_.find(stop.index_);
                        return it == cg.stop_states_.end() ||
                               it->second.state_ !=
                                   context::conn_graph_context::stop_state::
                                       Stop_completed;
                      }) == cg.cg_->stops_.end();
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
  return conn_graph.journeys_.at(stop.departure_infos_.back()
                                     .departing_journey_index_).j_;
}

module::msg_ptr to_routing_request(
    connection_graph& conn_graph, connection_graph::stop const& stop,
    unsigned int const num_failed_requests_before) {
  auto const time_begin = latest_departing_alternative(conn_graph, stop)
                              .stops.front()
                              .departure.timestamp +
                          60 + num_failed_requests_before * (15 * 60);
  auto const time_end = time_begin + (15 * 60);

  auto const stop_station = conn_graph.station_info(stop.index_);
  auto const arrival_station =
      conn_graph.station_info(connection_graph::stop::Index_arrival_stop);

  auto msg = flatbuffers_tools::to_routing_request(
      stop_station.first, stop_station.second, arrival_station.first,
      arrival_station.second, time_begin, time_end);
  // std::cout << "\n\nRequest:\n" << msg->to_json() << std::endl;
  return msg;
}

void build_cg(std::shared_ptr<context> c,
              context::conn_graph_context& conn_graph) {
  if (conn_graph.cg_state_ == context::conn_graph_context::CG_in_progress) {
    for (auto& stop : conn_graph.cg_->stops_) {
      auto it = conn_graph.stop_states_.find(stop.index_);
      if (it == conn_graph.stop_states_.end() ||
          it->second.state_ ==
              context::conn_graph_context::stop_state::Stop_idle) {
        it->second.state_ = context::conn_graph_context::stop_state::Stop_busy;
        return c->reliability_.send_message(
            to_routing_request(*conn_graph.cg_, stop,
                               it->second.num_failed_requests),
            c->session_, std::bind(&handle_alternative_response, p::_1, p::_2,
                                   c, conn_graph.index_, stop.index_));
      }
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
  // std::cout << "\n\nBase Response:\n" << msg->to_json() << std::endl;

  auto const lock = context->reliability_.synced_sched();
  schedule const& schedule = lock.sched();
  auto journeys = journey_builder::to_journeys(
      msg->content<routing::RoutingResponse const*>(), schedule.categories);
  if (journeys.empty()) {
    return build_result(Base_failed, context);
  }

  for (auto const& j : journeys) {
    context->connection_graphs_.emplace_back();
    auto& cg_context = context->connection_graphs_.back();
    cg_context.index_ = context->connection_graphs_.size() - 1;
    auto& conn_graph = *cg_context.cg_;
    connection_graph_builder::add_base_journey(conn_graph, j);
    /* todo: rate cg */

    for (auto const& stop : cg_context.cg_->stops_) {
      cg_context.stop_states_[stop.index_].state_ =
          context::conn_graph_context::stop_state::Stop_idle;
      cg_context.stop_states_[stop.index_].num_failed_requests = 0;
    }
    cg_context.stop_states_[connection_graph::stop::Index_departure_stop]
        .state_ = context::conn_graph_context::stop_state::Stop_completed;
    cg_context.stop_states_[connection_graph::stop::Index_arrival_stop].state_ =
        context::conn_graph_context::stop_state::Stop_completed;

    if (j.transfers == 0) {
      cg_context.cg_state_ = context::conn_graph_context::CG_completed;
    }
  }

  for (auto conn_graph : context->connection_graphs_) {
    build_cg(context, conn_graph);
  }
}

inline journey const& select_alternative(connection_graph const& conn_graph,
                                         std::vector<journey> const& journeys) {
  assert(!journeys.empty());
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
                                 unsigned int const conn_graph_idx,
                                 unsigned int const stop_idx) {
  if (e) {
    return build_result(CG_failed, c);
  }
  // std::cout << "\n\nAlternative Response:\n" << msg->to_json() << std::endl;

  auto& conn_graph = c->connection_graphs_.at(conn_graph_idx);
  auto& stop_state = conn_graph.stop_states_.at(stop_idx);

  auto const lock = c->reliability_.synced_sched();
  schedule const& schedule = lock.sched();
  auto journeys = journey_builder::to_journeys(
      msg->content<routing::RoutingResponse const*>(), schedule.categories);
  if (journeys.empty()) {
    ++stop_state.num_failed_requests;
    if (stop_state.num_failed_requests > 5) { /* todo */
      stop_state.state_ = context::conn_graph_context::stop_state::Stop_idle;
      return build_result(CG_failed, c);
    }
  } else {
    auto const& j = select_alternative(*conn_graph.cg_.get(), journeys);
    connection_graph_builder::add_alternative_journey(
        std::ref(*conn_graph.cg_.get()), stop_idx, j);
    /* todo: rate cg */
    stop_state.num_failed_requests = 0;
    if (c->f_(conn_graph.cg_->stops_.at(stop_idx), *conn_graph.cg_)) {
      stop_state.state_ =
          context::conn_graph_context::stop_state::Stop_completed;
      if (complete(conn_graph)) {
        conn_graph.cg_state_ =
            context::conn_graph_context::cg_state::CG_completed;
      }
    }
  }
  stop_state.state_ = context::conn_graph_context::stop_state::Stop_idle;
  build_cg(c, conn_graph);
}

void build_result(build_state state, std::shared_ptr<context> c) {
  if (c->result_returned_) {
    return;
  } else if (state == Base_failed || state == CG_failed) {
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
                motis::reliability::reliability& rel,
                motis::module::sid session, complete_func f, callback cb) {
  rel.send_message(
      flatbuffers_tools::to_flatbuffers_message(request->request()), session,
      std::bind(&detail::handle_base_response, p::_1, p::_2,
                std::make_shared<detail::context>(rel, session, cb, f)));
}

}  // namespace connection_graph_search
}  // namespace search
}  // namespace reliability
}  // namespace motis
