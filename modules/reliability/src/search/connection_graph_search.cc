#include <limits>

#include "motis/reliability/search/connection_graph_search.h"

#include "motis/core/schedule/schedule.h"

#include "motis/core/common/journey_builder.h"

#include "motis/protocol/RoutingResponse_generated.h"

#include "motis/reliability/error.h"
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
  context(motis::reliability::reliability& rel, schedule const& sched)
      : reliability_(rel), schedule_(sched) {}

  std::vector<connection_graph> connection_graphs_;
  motis::reliability::reliability& reliability_;
  schedule const& schedule_;
};

bool complete(connection_graph::stop const& stop) { return true; }
bool complete(connection_graph const& conn_graph) { return true; }
bool complete(context const& context) {
  for (auto const& cg : context.connection_graphs_) {
    if (!complete(cg)) {
      return false;
    }
  }
  return true;
}

journey const& latest_departing_alternative(
    connection_graph const& conn_graph, connection_graph::stop const& stop) {
  return std::find_if(conn_graph.journeys.rbegin(), conn_graph.journeys.rend(),
                      [stop](connection_graph::journey_info const& j) {
                        return stop.index == j.from_index;
                      })->j;
}

journey const& select_alternative(connection_graph const& conn_graph,
                                  std::vector<journey> const& journeys) {
  return journeys.front(); /* todo */
}

void search_alternative(connection_graph& conn_graph, context& context,
                        motis::module::sid session,
                        motis::module::callback cb) {
  for (auto& stop : conn_graph.stops) {
    if (!complete(stop)) {
      auto const& last_alternative =
          latest_departing_alternative(conn_graph, stop);
      time const time_begin = last_alternative.stops.back().arrival.timestamp;
      time const time_end = time_begin + (15 * 3600);
      auto const& arrival_stop =
          conn_graph.stops.at(connection_graph::stop::Index_arrival_stop);

      context.reliability_.send_message(
          flatbuffers_tools::to_routing_request(
              stop.name, stop.eva_no, arrival_stop.name, arrival_stop.eva_no,
              time_begin, time_end),
          session,
          std::bind(&handle_base_response, p::_1, p::_2, std::ref(conn_graph),
                    std::ref(context), session, cb));
    }
  }
}

void handle_base_response(motis::module::msg_ptr msg,
                          boost::system::error_code e,
                          connection_graph& conn_graph, context& context,
                          motis::module::sid session,
                          motis::module::callback cb) {
  if (e) {
    return cb(nullptr, e);
  }
  auto journeys = journey_builder::to_journeys(
      msg->content<routing::RoutingResponse const*>(),
      context.schedule_.categories);

  if (context.connection_graphs_.empty()) {
    for (auto const& j : journeys) {
      context.connection_graphs_.emplace_back();
      auto& conn_graph = context.connection_graphs_.back();
      connection_graph_builder::add_journey(conn_graph, j);
    }
    for (auto conn_graph : context.connection_graphs_) {
      if (!complete(conn_graph)) {
        search_alternative(conn_graph, context, session, cb);
      }
    }
  } else {
    auto const& j = select_alternative(conn_graph, journeys);
    connection_graph_builder::add_journey(conn_graph, j);
    if (!complete(conn_graph)) {
      search_alternative(conn_graph, context, session, cb);
    }
  }

  if (complete(context)) {
    cb({} /* todo: response */, error::ok);
  }
}
}  // namespace detail

void search_cgs(ReliableRoutingRequest const* request,
                motis::reliability::reliability& rel, schedule const& sched,
                motis::module::sid session, motis::module::callback cb) {
  connection_graph dummy;
  detail::context context(rel, sched);
  rel.send_message(
      flatbuffers_tools::to_flatbuffers_message(request->request()), session,
      std::bind(&detail::handle_base_response, p::_1, p::_2, std::ref(dummy),
                std::ref(context), session, cb));
}

}  // namespace connection_graph_search
}  // namespace search
}  // namespace reliability
}  // namespace motis
