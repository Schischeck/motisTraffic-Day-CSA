#include "motis/reliability/tools/flatbuffers/request_builder.h"

#include "motis/reliability/tools/hotels.h"

using namespace flatbuffers;

namespace motis {
namespace reliability {
namespace flatbuffers {
namespace request_builder {

namespace detail {
time_t to_unix_time(std::tuple<int, int, int> ddmmyyyy, motis::time time) {
  return motis_to_unixtime(
      motis::to_unix_time(std::get<2>(ddmmyyyy), std::get<1>(ddmmyyyy),
                          std::get<0>(ddmmyyyy)),
      time);
}
module::msg_ptr to_reliable_routing_request(
    module::MessageCreator& b, std::string const& from_name,
    std::string const& from_eva, std::string const& to_name,
    std::string const& to_eva, motis::time interval_begin,
    motis::time interval_end, std::tuple<int, int, int> ddmmyyyy,
    Offset<RequestOptionsWrapper>& request_type_wrapper,
    std::vector<Offset<routing::AdditionalEdgeWrapper>> const&
        additional_edges) {
  std::vector<Offset<routing::StationPathElement>> station_elements;
  station_elements.push_back(routing::CreateStationPathElement(
      b, b.CreateString(from_name), b.CreateString(from_eva)));
  station_elements.push_back(routing::CreateStationPathElement(
      b, b.CreateString(to_name), b.CreateString(to_eva)));
  routing::Interval interval(to_unix_time(ddmmyyyy, interval_begin),
                             to_unix_time(ddmmyyyy, interval_end));
  b.CreateAndFinish(MsgContent_ReliableRoutingRequest,
                    reliability::CreateReliableRoutingRequest(
                        b, routing::CreateRoutingRequest(
                               b, &interval, routing::Type::Type_PreTrip,
                               routing::Direction::Direction_Forward,
                               b.CreateVector(station_elements),
                               b.CreateVector(additional_edges)),
                        request_type_wrapper)
                        .Union());
  return module::make_msg(b);
}
module::msg_ptr to_reliable_routing_request(
    module::MessageCreator& b, std::string const& from_name,
    std::string const& from_eva, std::string const& to_name,
    std::string const& to_eva, motis::time interval_begin,
    motis::time interval_end, std::tuple<int, int, int> ddmmyyyy,
    Offset<RequestOptionsWrapper>& request_type_wrapper) {
  std::vector<Offset<routing::AdditionalEdgeWrapper>> dummy;
  return to_reliable_routing_request(b, from_name, from_eva, to_name, to_eva,
                                     interval_begin, interval_end, ddmmyyyy,
                                     request_type_wrapper, dummy);
}
}

module::msg_ptr to_flatbuffers_message(routing::RoutingRequest const* request) {
  /* convert routing::RoutingRequest to Offset<RoutingRequest> */
  module::MessageCreator b;
  b.ForceDefaults(true); /* necessary to write indices 0 */
  std::vector<Offset<routing::StationPathElement>> station_elements;
  for (auto it = request->path()->begin(); it != request->path()->end(); ++it) {
    std::string const name = it->name() ? it->name()->c_str() : "";
    station_elements.push_back(
        it->eva_nr()
            ? routing::CreateStationPathElement(
                  b, b.CreateString(name),
                  b.CreateString(it->eva_nr()->c_str()))
            : routing::CreateStationPathElement(b, b.CreateString(name)));
  }
  routing::Interval interval(request->interval()->begin(),
                             request->interval()->end());
  std::vector<Offset<routing::AdditionalEdgeWrapper>> dummy;
  b.CreateAndFinish(MsgContent_RoutingRequest,
                    routing::CreateRoutingRequest(
                        b, &interval, request->type(), request->direction(),
                        b.CreateVector(station_elements), b.CreateVector(dummy))
                        .Union());
  return module::make_msg(b);
}

module::msg_ptr to_routing_request(std::string const& from_name,
                                   std::string const& from_eva,
                                   std::string const& to_name,
                                   std::string const& to_eva,
                                   time_t interval_begin, time_t interval_end,
                                   bool const ontrip) {
  module::MessageCreator b;
  b.ForceDefaults(true); /* necessary to write indices 0 */
  std::vector<Offset<routing::StationPathElement>> station_elements;
  station_elements.push_back(routing::CreateStationPathElement(
      b, b.CreateString(from_name), b.CreateString(from_eva)));
  station_elements.push_back(routing::CreateStationPathElement(
      b, b.CreateString(to_name), b.CreateString(to_eva)));
  routing::Interval interval(interval_begin, interval_end);
  std::vector<Offset<routing::AdditionalEdgeWrapper>> dummy;
  b.CreateAndFinish(MsgContent_RoutingRequest,
                    routing::CreateRoutingRequest(
                        b, &interval, (ontrip ? routing::Type::Type_OnTrip
                                              : routing::Type::Type_PreTrip),
                        routing::Direction::Direction_Forward,
                        b.CreateVector(station_elements), b.CreateVector(dummy))
                        .Union());
  return module::make_msg(b);
}

module::msg_ptr to_routing_request(
    std::string const& from_name, std::string const& from_eva,
    std::string const& to_name, std::string const& to_eva,
    motis::time interval_begin, motis::time interval_end,
    std::tuple<int, int, int> ddmmyyyy, bool const ontrip) {
  return to_routing_request(from_name, from_eva, to_name, to_eva,
                            detail::to_unix_time(ddmmyyyy, interval_begin),
                            detail::to_unix_time(ddmmyyyy, interval_end),
                            ontrip);
}

module::msg_ptr to_reliable_routing_request(
    std::string const& from_name, std::string const& from_eva,
    std::string const& to_name, std::string const& to_eva,
    motis::time interval_begin, motis::time interval_end,
    std::tuple<int, int, int> ddmmyyyy, short const min_dep_diff) {
  module::MessageCreator b;
  b.ForceDefaults(true); /* necessary to write indices 0 */
  auto request_type_wrapper = reliability::CreateRequestOptionsWrapper(
      b, reliability::RequestOptions_ReliableSearchReq,
      reliability::CreateReliableSearchReq(b, min_dep_diff).Union());
  return detail::to_reliable_routing_request(
      b, from_name, from_eva, to_name, to_eva, interval_begin, interval_end,
      ddmmyyyy, request_type_wrapper);
}

module::msg_ptr to_rating_request(std::string const& from_name,
                                  std::string const& from_eva,
                                  std::string const& to_name,
                                  std::string const& to_eva,
                                  motis::time interval_begin,
                                  motis::time interval_end,
                                  std::tuple<int, int, int> ddmmyyyy) {
  module::MessageCreator b;
  b.ForceDefaults(true); /* necessary to write indices 0 */
  auto request_type_wrapper = reliability::CreateRequestOptionsWrapper(
      b, reliability::RequestOptions_RatingReq,
      reliability::CreateRatingReq(b).Union());
  return detail::to_reliable_routing_request(
      b, from_name, from_eva, to_name, to_eva, interval_begin, interval_end,
      ddmmyyyy, request_type_wrapper);
}

module::msg_ptr to_connection_tree_request(
    std::string const& from_name, std::string const& from_eva,
    std::string const& to_name, std::string const& to_eva,
    motis::time interval_begin, motis::time interval_end,
    std::tuple<int, int, int> ddmmyyyy, short const num_alternatives_at_stop,
    short const min_dep_diff) {
  module::MessageCreator b;
  b.ForceDefaults(true); /* necessary to write indices 0 */
  auto request_type_wrapper = reliability::CreateRequestOptionsWrapper(
      b, reliability::RequestOptions_ConnectionTreeReq,
      reliability::CreateConnectionTreeReq(b, num_alternatives_at_stop,
                                           min_dep_diff)
          .Union());
  return detail::to_reliable_routing_request(
      b, from_name, from_eva, to_name, to_eva, interval_begin, interval_end,
      ddmmyyyy, request_type_wrapper);
}

module::msg_ptr to_late_connections_request(
    std::string const& from_name, std::string const& from_eva,
    std::string const& to_name, std::string const& to_eva,
    motis::time interval_begin, motis::time interval_end,
    std::tuple<int, int, int> ddmmyyyy,
    /* taxi-info: from-station, duration, price */
    std::vector<std::tuple<std::string, unsigned short, unsigned short>> const&
        taxi_infos) {
  using namespace routing;
  module::MessageCreator b;
  b.ForceDefaults(true); /* necessary to write indices 0 */
  auto request_type_wrapper = reliability::CreateRequestOptionsWrapper(
      b, reliability::RequestOptions_LateConnectionReq,
      reliability::CreateLateConnectionReq(b).Union());

  std::vector<Offset<AdditionalEdgeWrapper>> additional_edges;
  for (auto const& info : taxi_infos) {
    additional_edges.push_back(CreateAdditionalEdgeWrapper(
        b, AdditionalEdge_MumoEdge,
        CreateMumoEdge(b, b.CreateString(std::get<0>(info)),
                       b.CreateString("-2") /* to dummy target */,
                       std::get<1>(info), std::get<2>(info))
            .Union()));
  }

  return detail::to_reliable_routing_request(
      b, from_name, from_eva, to_name, to_eva, interval_begin, interval_end,
      ddmmyyyy, request_type_wrapper, additional_edges);
}

module::msg_ptr to_late_connections_routing_request(
    routing::RoutingRequest const* request,
    std::vector<hotels::hotel_info> const& hotel_infos) {
  /* convert routing::RoutingRequest to Offset<RoutingRequest> */
  using namespace routing;
  module::MessageCreator b;
  b.ForceDefaults(true); /* necessary to write indices 0 */
  std::vector<Offset<StationPathElement>> station_elements;
  for (auto it = request->path()->begin(); it != request->path()->end(); ++it) {
    std::string const name = it->name() ? it->name()->c_str() : "";
    station_elements.push_back(
        it->eva_nr()
            ? CreateStationPathElement(b, b.CreateString(name),
                                       b.CreateString(it->eva_nr()->c_str()))
            : CreateStationPathElement(b, b.CreateString(name)));
  }
  Interval interval(request->interval()->begin(), request->interval()->end());

  std::vector<Offset<AdditionalEdgeWrapper>> additional_edges;
  for (auto it = request->additional_edges()->begin();
       it != request->additional_edges()->end(); ++it) {
    auto const* mumo = (MumoEdge const*)it->additional_edge();
    additional_edges.push_back(CreateAdditionalEdgeWrapper(
        b, AdditionalEdge_MumoEdge,
        CreateMumoEdge(b, b.CreateString(mumo->from_station_eva()->c_str()),
                       b.CreateString(mumo->to_station_eva()->c_str()),
                       mumo->duration(), mumo->price())
            .Union()));
  }
  for (auto const& hotel : hotel_infos) {
    additional_edges.push_back(CreateAdditionalEdgeWrapper(
        b, AdditionalEdge_HotelEdge,
        CreateHotelEdge(b, b.CreateString(hotel.station_),
                        hotel.earliest_checkout_, hotel.min_stay_duration_,
                        hotel.price_)
            .Union()));
  }

  b.CreateAndFinish(MsgContent_RoutingRequest,
                    CreateRoutingRequest(b, &interval, Type::Type_OnTrip,
                                         request->direction(),
                                         b.CreateVector(station_elements),
                                         b.CreateVector(additional_edges))
                        .Union());
  return module::make_msg(b);
}

}  // namespace request_builder
}  // namespace flatbuffers
}  // namespace reliability
}  // namespace motis
