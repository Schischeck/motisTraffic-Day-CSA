#include "motis/reliability/search/cg_search_context.h"
#include "motis/reliability/tools/flatbuffers/request_builder.h"

namespace motis {
namespace reliability {
namespace search {
namespace connection_graph_search {
namespace tools {

inline void output(journey const& j, schedule const& sched, std::ostream& os) {
  os << "Journey " << j.duration_ << "minutes:" << std::endl;
  for (auto const& t : j.transports_) {
    auto const& from = j.stops_[t.from_];
    auto const& to = j.stops_[t.to_];
    if (from.name_ != "DUMMY" && to.name_ != "DUMMY") {
      os << from.name_ << "("
         << format_time(unix_to_motistime(sched.schedule_begin_,
                                          from.departure_.timestamp_))
         << ") --" << t.category_name_ << t.train_nr_ << "-> ("
         << format_time(unix_to_motistime(sched.schedule_begin_,
                                          to.arrival_.timestamp_))
         << ") " << to.name_ << std::endl;
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

inline journey const& latest_departing_alternative(
    connection_graph const& conn_graph, connection_graph::stop const& stop) {
  return conn_graph.journeys_.at(stop.alternative_infos_.back().journey_index_);
}

inline std::pair<module::msg_ptr, detail::context::journey_cache_key>
to_routing_request(connection_graph& conn_graph,
                   connection_graph::stop const& stop,
                   duration const min_departure_diff) {
  auto const ontrip_time = latest_departing_alternative(conn_graph, stop)
                               .stops_.front()
                               .departure_.timestamp_ +
                           min_departure_diff * 60;
  auto const stop_station = conn_graph.station_info(stop.index_);
  auto const arrival_station =
      conn_graph.station_info(connection_graph::stop::Index_arrival_stop);
  auto msg = flatbuffers::request_builder(routing::Type::Type_OnTrip)
                 .add_station(stop_station.first, stop_station.second)
                 .add_station(arrival_station.first, arrival_station.second)
                 .set_interval(ontrip_time, ontrip_time)
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
  if (j.transports_.size() < 1 || j.stops_.size() < 2) {
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
  for (auto it = journeys.begin(); it != journeys.end(); ++it) {
    if (check_journey(*it)) {
      filtered.push_back(std::move(*it));
    }
  }
  return filtered;
}

}  // namespace tools
}  // namespace connection_graph_search
}  // namespace search
}  // namespace reliability
}  // namespace motis
