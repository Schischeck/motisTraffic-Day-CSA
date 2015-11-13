#include "motis/reliability/search/cg_search_context.h"
#include "motis/reliability/tools/flatbuffers_tools.h"

namespace motis {
namespace reliability {
namespace search {
namespace connection_graph_search {
namespace tools {

inline void output(journey const& j, schedule const& sched, std::ostream& os) {
  os << "Journey " << j.duration << "minutes:" << std::endl;
  for (auto const& t : j.transports) {
    auto const& from = j.stops[t.from];
    auto const& to = j.stops[t.to];
    if (from.name != "DUMMY" && to.name != "DUMMY") {
      os << from.name << "("
         << format_time(unix_to_motistime(sched.schedule_begin_,
                                          from.departure.timestamp))
         << ") --" << t.category_name << t.train_nr << "-> ("
         << format_time(
                unix_to_motistime(sched.schedule_begin_, to.arrival.timestamp))
         << ") " << to.name << std::endl;
    }
  }
}

inline bool complete(detail::context::conn_graph_context const& cg) {
  return std::find_if(cg.cg_->stops_.begin(), cg.cg_->stops_.end(),
                      [&](connection_graph::stop const& stop) {
                        auto const it = cg.stop_states_.find(stop.index_);
                        return it == cg.stop_states_.end() ||
                               it->second.state_ !=
                                   detail::context::conn_graph_context::
                                       stop_state::Stop_completed;
                      }) == cg.cg_->stops_.end();
}
inline bool complete(detail::context const& c) {
  return std::find_if(
             c.connection_graphs_.begin(), c.connection_graphs_.end(),
             [&](detail::context::conn_graph_context const& cg) {
               return cg.cg_state_ ==
                      detail::context::conn_graph_context::CG_in_progress;
             }) == c.connection_graphs_.end();
}

inline journey const& latest_departing_alternative(
    connection_graph const& conn_graph, connection_graph::stop const& stop) {
  return conn_graph.journeys_.at(
      stop.alternative_infos_.back().journey_index_);
}

inline std::pair<module::msg_ptr, detail::context::journey_cache_key>
to_routing_request(connection_graph& conn_graph,
                   connection_graph::stop const& stop,
                   duration const min_departure_diff) {
  auto const time_begin = latest_departing_alternative(conn_graph, stop)
                              .stops.front()
                              .departure.timestamp +
                          min_departure_diff * 60;
  auto const time_end = time_begin;

  auto const stop_station = conn_graph.station_info(stop.index_);
  auto const arrival_station =
      conn_graph.station_info(connection_graph::stop::Index_arrival_stop);

  auto msg = flatbuffers_tools::to_routing_request(
      stop_station.first, stop_station.second, arrival_station.first,
      arrival_station.second, time_begin, time_end, true);
  return std::make_pair(msg, detail::context::journey_cache_key(
                                 stop_station.second, time_begin, time_end));
}

inline journey const& select_alternative(std::vector<journey> const& journeys) {
  assert(!journeys.empty());
  /* earliest arrival */
  return std::ref(*std::min_element(journeys.begin(), journeys.end(),
                                    [](journey const& a, journey const& b) {
                                      return a.stops.back().arrival.timestamp <
                                             b.stops.back().arrival.timestamp;
                                    }));
}

}  // namespace tools
}  // namespace connection_graph_search
}  // namespace search
}  // namespace reliability
}  // namespace motis
