#include <limits>
#include <memory>

#include "motis/reliability/search/connection_graph_search.h"

#include "motis/core/schedule/schedule.h"
#include "motis/core/schedule/time.h"

#include "motis/core/common/journey_builder.h"

#include "motis/protocol/RoutingResponse_generated.h"

#include "motis/reliability/reliability.h"
#include "motis/reliability/search/connection_graph.h"
#include "motis/reliability/search/connection_graph_builder.h"
#include "motis/reliability/search/simple_connection_graph_optimizer.h"
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
      enum state {
        Stop_idle,
        Stop_busy,
        Stop_completed,
        Stop_Aborted
      } state_ = Stop_idle;
      unsigned short num_failed_requests = 0;
    };
    std::map<unsigned int, stop_state> stop_states_;
  };
  context(motis::reliability::reliability& rel, motis::module::sid session,
          callback cb, connection_graph_optimizer const& optimizer)
      : reliability_(rel),
        session_(session),
        result_callback_(cb),
        optimizer_(optimizer),
        result_returned_(false) {}

  std::vector<conn_graph_context> connection_graphs_;
  motis::reliability::reliability& reliability_;
  motis::module::sid session_;
  callback result_callback_;
  connection_graph_optimizer const& optimizer_;
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

void output(journey const& j, schedule const& sched, std::ostream& os) {
  os << "Journey " << j.duration << "minutes:" << std::endl;
  for (auto const& t : j.transports) {
    auto const& from = j.stops[t.from];
    auto const& to = j.stops[t.to];
    if (from.name != "DUMMY" && to.name != "DUMMY") {
      os << from.name << "("
         << unix_to_motistime(sched.schedule_begin_, from.departure.timestamp)
         << ") --" << t.category_name << t.train_nr << "-> ("
         << unix_to_motistime(sched.schedule_begin_, to.arrival.timestamp)
         << ") " << to.name << std::endl;
    }
  }
}

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
  duration const min_departure_diff = 60;
  duration const max_departure_diff = 15 * 60;
  auto const time_begin = latest_departing_alternative(conn_graph, stop)
                              .stops.front()
                              .departure.timestamp +
                          min_departure_diff +
                          num_failed_requests_before * max_departure_diff;
  auto const time_end = time_begin + max_departure_diff;

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
  std::cout << "\nbuild cg " << conn_graph.index_ << std::endl;
  if (conn_graph.cg_state_ == context::conn_graph_context::CG_in_progress) {
    for (auto& stop : conn_graph.cg_->stops_) {
      auto it = conn_graph.stop_states_.find(stop.index_);
      if (it == conn_graph.stop_states_.end() ||
          it->second.state_ ==
              context::conn_graph_context::stop_state::Stop_idle) {
        it->second.state_ = context::conn_graph_context::stop_state::Stop_busy;
        std::cout << "Stop " << conn_graph.cg_->station_info(stop.index_).first
                  << " has already " << stop.departure_infos_.size()
                  << " alternatives" << std::endl;
        return c->reliability_.send_message(
            to_routing_request(*conn_graph.cg_, stop,
                               it->second.num_failed_requests),
            c->session_, std::bind(&handle_alternative_response, p::_1, p::_2,
                                   c, conn_graph.index_, stop.index_));
      }
    }
    std::cout << "\nNo idle stop found!" << std::endl;
  } else {
    return build_result(CG_completed, c);
  }
  /* unexpected case */
  return build_result(CG_failed, c);
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
  // std::cout << "\n\nBase Response:\n" << msg->to_json() << std::endl;
  if (e) {
    return build_result(Base_failed, context);
  }

  auto const lock = context->reliability_.synced_sched();
  schedule const& schedule = lock.sched();
  auto journeys = journey_builder::to_journeys(
      msg->content<routing::RoutingResponse const*>(), schedule.categories);
  if (journeys.empty()) {
    return build_result(Base_failed, context);
  }

  std::cout << "\n\n-----------------------\n" << journeys.size()
            << " base journeys found" << std::endl;

  for (auto const& j : journeys) {
    output(j, context->reliability_.synced_sched().sched(), std::cout);
    context->connection_graphs_.emplace_back();
    auto& cg_context = context->connection_graphs_.back();
    cg_context.index_ = context->connection_graphs_.size() - 1;
    auto& conn_graph = *cg_context.cg_;
    connection_graph_builder::add_base_journey(conn_graph, j);
    /* todo: rate cg */

    insert_stop_states(*context, cg_context);

    if (conn_graph.journeys_.size() == 1 || complete(cg_context)) {
      cg_context.cg_state_ = context::conn_graph_context::CG_completed;
    } else {
      cg_context.cg_state_ = context::conn_graph_context::CG_in_progress;
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
  std::cout << "\n\n-----------------------\nhandle_alternative_response of cg "
            << conn_graph_idx << " at stop " << stop_idx << "("
            << c->connection_graphs_.at(conn_graph_idx)
                   .cg_->station_info(stop_idx)
                   .first << ")" << std::endl;
  if (e) {
    return build_result(CG_failed, c);
  }
  // std::cout << "\n\nAlternative Response:\n" << msg->to_json() << std::endl;

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
      std::cout << "\nsent more than 5 requests" << std::endl;
    }
  } else {
    auto const& j = select_alternative(*conn_graph.cg_.get(), journeys);
    output(j, c->reliability_.synced_sched().sched(), std::cout);
    connection_graph_builder::add_alternative_journey(
        std::ref(*conn_graph.cg_.get()), stop_idx, j);
    /* todo: rate cg */
    stop_state.num_failed_requests = 0;

    if (conn_graph.stop_states_.size() < conn_graph.cg_->stops_.size()) {
      insert_stop_states(*c, conn_graph);
    }
  }

  std::cout << "Stop " << stop_idx << "("
            << conn_graph.cg_->station_info(stop_idx).first << ") of cg "
            << conn_graph_idx << std::flush;
  if (c->optimizer_.complete(conn_graph.cg_->stops_.at(stop_idx),
                             *conn_graph.cg_)) {
    std::cout << " is completed with "
              << conn_graph.cg_->stops_.at(stop_idx).departure_infos_.size()
              << " alternatives." << std::endl;
    stop_state.state_ = context::conn_graph_context::stop_state::Stop_completed;
    if (complete(conn_graph)) {
      conn_graph.cg_state_ =
          context::conn_graph_context::cg_state::CG_completed;
    }
  } else {
    std::cout << " has only "
              << conn_graph.cg_->stops_.at(stop_idx).departure_infos_.size()
              << " alternatives" << std::endl;
    stop_state.state_ = context::conn_graph_context::stop_state::Stop_idle;
  }

  build_cg(c, conn_graph);
}

void build_result(build_state state, std::shared_ptr<context> c) {
  std::cout << "\nbuild_result " << state << " " << std::flush;
  for (auto const& cg : c->connection_graphs_)
    std::cout << cg.cg_state_ << " " << std::flush;
  std::cout << std::endl;

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
