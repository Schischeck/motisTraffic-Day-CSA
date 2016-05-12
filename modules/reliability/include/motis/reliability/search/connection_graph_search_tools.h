#include "motis/reliability/search/cg_search_context.h"
#include "motis/reliability/tools/flatbuffers/request_builder.h"

namespace motis {
namespace reliability {
namespace search {
namespace connection_graph_search {
namespace tools {

inline journey const& latest_departing_alternative(
    connection_graph const& conn_graph, connection_graph::stop const& stop) {
  return conn_graph.journeys_.at(stop.alternative_infos_.back().journey_index_);
}

inline std::pair<module::msg_ptr, detail::context::journey_cache_key>
to_routing_request(connection_graph const& conn_graph,
                   connection_graph::stop const& stop,
                   duration const min_departure_diff) {
  auto const ontrip_time = latest_departing_alternative(conn_graph, stop)
                               .stops_.front()
                               .departure_.timestamp_ +
                           min_departure_diff * 60;
  auto const stop_station = conn_graph.station_info(stop.index_);
  auto const arrival_station =
      conn_graph.station_info(connection_graph::stop::INDEX_ARRIVAL_STOP);
  auto msg = flatbuffers::request_builder()
                 .add_ontrip_station_start(stop_station.first,
                                           stop_station.second, ontrip_time)
                 .add_destination(arrival_station.first, arrival_station.second)
                 .build_routing_request();
  return std::make_pair(msg, detail::context::journey_cache_key(
                                 stop_station.second, ontrip_time));
}

inline journey const& select_alternative(std::vector<journey> const& journeys) {
  assert(!journeys.empty());
  /* earliest arrival */
  return std::ref(*std::min_element(
      journeys.begin(), journeys.end(), [](journey const& a, journey const& b) {
        return a.stops_.back().arrival_.timestamp_ <
               b.stops_.back().arrival_.timestamp_;
      }));
}

inline bool check_journey(journey const& j) {
  if (j.transports_.empty() || j.stops_.size() < 2) {
    return false;
  }
  if (j.stops_.front().departure_.timestamp_ == 0 ||
      j.stops_.back().arrival_.timestamp_ == 0 ||
      j.stops_.back().arrival_.timestamp_ <
          j.stops_.front().departure_.timestamp_) {
    return false;
  }
  return true;
}

inline std::vector<journey> remove_invalid_journeys(
    std::vector<journey> const& journeys) {
  std::vector<journey> filtered;
  for (auto& j : journeys) {
    if (check_journey(j)) {
      filtered.push_back(j);
    }
  }
  return filtered;
}

}  // namespace tools
}  // namespace connection_graph_search
}  // namespace search
}  // namespace reliability
}  // namespace motis
