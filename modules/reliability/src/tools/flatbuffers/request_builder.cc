#include "motis/reliability/tools/flatbuffers/request_builder.h"

#include "motis/reliability/tools/hotels.h"

using namespace flatbuffers;
using namespace motis::module;

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

struct request_builder {
  request_builder(
      routing::Type const type = routing::Type::Type_PreTrip,
      routing::Direction const dir = routing::Direction::Direction_Forward)
      : type_(type), direction_(dir) {
    b_.ForceDefaults(true); /* necessary to write indices 0 */
  }

  request_builder(routing::RoutingRequest const* request)
      : request_builder(request->type(), request->direction()) {
    set_interval(request->interval()->begin(), request->interval()->end());
    for (auto const& e : *request->path()) {
      add_station(e->name() ? e->name()->c_str() : "",
                  e->eva_nr() ? e->eva_nr()->c_str() : "");
    }
  }

  request_builder& add_station(std::string const& name,
                               std::string const& eva) {
    stations_.push_back(routing::CreateStationPathElement(
        b_, b_.CreateString(name), b_.CreateString(eva)));
    return *this;
  }

  request_builder& set_interval(std::time_t const begin,
                                std::time_t const end) {
    interval_begin_ = begin;
    interval_end_ = end;
    return *this;
  }

  request_builder& set_interval(std::tuple<int, int, int> const ddmmyyyy,
                                motis::time const begin,
                                motis::time const end) {
    interval_begin_ = detail::to_unix_time(ddmmyyyy, begin);
    interval_end_ = detail::to_unix_time(ddmmyyyy, end);
    return *this;
  }

  request_builder& add_additional_edge(
      Offset<routing::AdditionalEdgeWrapper> const& edge) {
    additional_edges_.push_back(edge);
    return *this;
  }

  Offset<routing::RoutingRequest> create_routing_request() {
    routing::Interval interval(interval_begin_, interval_end_);
    return routing::CreateRoutingRequest(b_, &interval, type_, direction_,
                                         b_.CreateVector(stations_),
                                         b_.CreateVector(additional_edges_));
  }

  msg_ptr build_routing_request() {
    b_.CreateAndFinish(MsgContent_RoutingRequest,
                       create_routing_request().Union());
    return module::make_msg(b_);
  }

  msg_ptr build_search_request(short const min_dep_diff) {
    auto opts = reliability::CreateRequestOptionsWrapper(
        b_, reliability::RequestOptions_ReliableSearchReq,
        reliability::CreateReliableSearchReq(b_, min_dep_diff).Union());
    return build_reliable_request(opts);
  }

  msg_ptr build_rating_request() {
    auto opts = reliability::CreateRequestOptionsWrapper(
        b_, reliability::RequestOptions_RatingReq,
        reliability::CreateRatingReq(b_).Union());
    return build_reliable_request(opts);
  }

  msg_ptr build_late_connection_cequest() {
    auto opts = reliability::CreateRequestOptionsWrapper(
        b_, reliability::RequestOptions_LateConnectionReq,
        reliability::CreateLateConnectionReq(b_).Union());
    return build_reliable_request(opts);
  }

  msg_ptr build_connection_tree_request(short const num_alternatives_at_stop,
                                        short const min_dep_diff) {
    auto opts = reliability::CreateRequestOptionsWrapper(
        b_, reliability::RequestOptions_ConnectionTreeReq,
        reliability::CreateConnectionTreeReq(b_, num_alternatives_at_stop,
                                             min_dep_diff)
            .Union());
    return build_reliable_request(opts);
  }

  msg_ptr build_reliable_request(Offset<RequestOptionsWrapper> const& options) {
    b_.CreateAndFinish(MsgContent_ReliableRoutingRequest,
                       reliability::CreateReliableRoutingRequest(
                           b_, create_routing_request(), options)
                           .Union());
    return module::make_msg(b_);
  }

  module::MessageCreator b_;
  routing::Type type_;
  routing::Direction direction_;
  std::vector<Offset<routing::StationPathElement>> stations_;
  std::time_t interval_begin_, interval_end_;
  std::vector<Offset<routing::AdditionalEdgeWrapper>> additional_edges_;
};

}  // namespace detail

// convert routing::RoutingRequest to Offset<RoutingRequest>
msg_ptr to_flatbuffers_message(routing::RoutingRequest const* request) {
  return detail::request_builder(request).build_routing_request();
}

module::msg_ptr to_routing_request(std::string const& from_name,
                                   std::string const& from_eva,
                                   std::string const& to_name,
                                   std::string const& to_eva,
                                   time_t interval_begin, time_t interval_end,
                                   bool const ontrip) {
  return detail::request_builder(ontrip ? routing::Type::Type_OnTrip
                                        : routing::Type::Type_PreTrip)
      .add_station(from_name, from_eva)
      .add_station(to_name, to_eva)
      .set_interval(interval_begin, interval_end)
      .build_routing_request();
}

module::msg_ptr to_routing_request(
    std::string const& from_name, std::string const& from_eva,
    std::string const& to_name, std::string const& to_eva,
    motis::time interval_begin, motis::time interval_end,
    std::tuple<int, int, int> ddmmyyyy, bool const ontrip) {
  return detail::request_builder(ontrip ? routing::Type::Type_OnTrip
                                        : routing::Type::Type_PreTrip)
      .add_station(from_name, from_eva)
      .add_station(to_name, to_eva)
      .set_interval(ddmmyyyy, interval_begin, interval_end)
      .build_routing_request();
}

module::msg_ptr to_reliable_routing_request(
    std::string const& from_name, std::string const& from_eva,
    std::string const& to_name, std::string const& to_eva,
    motis::time interval_begin, motis::time interval_end,
    std::tuple<int, int, int> ddmmyyyy, short const min_dep_diff) {
  return detail::request_builder()
      .add_station(from_name, from_eva)
      .add_station(to_name, to_eva)
      .set_interval(ddmmyyyy, interval_begin, interval_end)
      .build_search_request(min_dep_diff);
}

module::msg_ptr to_reliable_routing_request(std::string const& from_name,
                                            std::string const& from_eva,
                                            std::string const& to_name,
                                            std::string const& to_eva,
                                            std::time_t interval_begin,
                                            std::time_t interval_end,
                                            short const min_dep_diff) {
  return detail::request_builder()
      .add_station(from_name, from_eva)
      .add_station(to_name, to_eva)
      .set_interval(interval_begin, interval_end)
      .build_search_request(min_dep_diff);
}

module::msg_ptr to_rating_request(std::string const& from_name,
                                  std::string const& from_eva,
                                  std::string const& to_name,
                                  std::string const& to_eva,
                                  motis::time interval_begin,
                                  motis::time interval_end,
                                  std::tuple<int, int, int> ddmmyyyy) {
  return detail::request_builder()
      .add_station(from_name, from_eva)
      .add_station(to_name, to_eva)
      .set_interval(ddmmyyyy, interval_begin, interval_end)
      .build_rating_request();
}

module::msg_ptr to_connection_tree_request(
    std::string const& from_name, std::string const& from_eva,
    std::string const& to_name, std::string const& to_eva,
    motis::time interval_begin, motis::time interval_end,
    std::tuple<int, int, int> ddmmyyyy, short const num_alternatives_at_stop,
    short const min_dep_diff) {
  return detail::request_builder()
      .add_station(from_name, from_eva)
      .add_station(to_name, to_eva)
      .set_interval(ddmmyyyy, interval_begin, interval_end)
      .build_connection_tree_request(num_alternatives_at_stop, min_dep_diff);
}

/* taxi-info: from-station, duration, price */
using taxi_info = std::tuple<std::string, unsigned short, unsigned short>;

module::msg_ptr to_reliable_late_connections_request(
    std::string const& from_name, std::string const& from_eva,
    std::string const& to_name, std::string const& to_eva,
    motis::time interval_begin, motis::time interval_end,
    std::tuple<int, int, int> ddmmyyyy,
    std::vector<taxi_info> const& taxi_infos) {
  using namespace routing;

  detail::request_builder builder;
  builder.add_station(from_name, from_eva)
      .add_station(to_name, to_eva)
      .set_interval(ddmmyyyy, interval_begin, interval_end);

  auto& b = builder.b_;
  for (auto const& info : taxi_infos) {
    builder.add_additional_edge(CreateAdditionalEdgeWrapper(
        b, AdditionalEdge_TimeDependentMumoEdge,
        CreateTimeDependentMumoEdge(
            b, CreateMumoEdge(b, b.CreateString(std::get<0>(info)),
                              b.CreateString("-2") /* to dummy target */,
                              std::get<1>(info), std::get<2>(info)),
            21 * 60, 3 * 60)
            .Union()));
  }

  return builder.build_late_connection_cequest();
}

/* convert routing::RoutingRequest to Offset<RoutingRequest> */
module::msg_ptr to_routing_late_connections_message(
    routing::RoutingRequest const* request,
    std::vector<hotels::hotel_info> const& hotel_infos) {
  using namespace routing;
  detail::request_builder builder(request);
  builder.type_ = Type::Type_LateConnection;

  auto& b = builder.b_;
  for (auto const& e : *request->additional_edges()) {
    if (e->additional_edge_type() != AdditionalEdge_TimeDependentMumoEdge) {
      continue;
    }
    auto const* mumo =
        static_cast<TimeDependentMumoEdge const*>(e->additional_edge());
    builder.add_additional_edge(CreateAdditionalEdgeWrapper(
        b, AdditionalEdge_TimeDependentMumoEdge,
        CreateTimeDependentMumoEdge(
            b, CreateMumoEdge(
                   b, b.CreateString(mumo->edge()->from_station_eva()->c_str()),
                   b.CreateString(mumo->edge()->to_station_eva()->c_str()),
                   mumo->edge()->duration(), mumo->edge()->price()),
            21 * 60, 3 * 60)
            .Union()));
  }
  for (auto const& hotel : hotel_infos) {
    builder.add_additional_edge(CreateAdditionalEdgeWrapper(
        b, AdditionalEdge_HotelEdge,
        CreateHotelEdge(b, b.CreateString(hotel.station_),
                        hotel.earliest_checkout_, hotel.min_stay_duration_,
                        hotel.price_)
            .Union()));
  }

  return builder.build_routing_request();
}

module::msg_ptr to_bikesharing_request(double const departure_lat,
                                       double const departure_lng,
                                       double const arrival_lat,
                                       double const arrival_lng,
                                       time_t window_begin, time_t window_end) {
  module::MessageCreator fb;
  fb.CreateAndFinish(
      MsgContent_BikesharingRequest,
      bikesharing::CreateBikesharingRequest(
          fb, departure_lat, departure_lng, arrival_lat, arrival_lng,
          window_begin, window_end, bikesharing::AvailabilityAggregator_Average)
          .Union());
  return module::make_msg(fb);
}

}  // namespace request_builder
}  // namespace flatbuffers
}  // namespace reliability
}  // namespace motis
