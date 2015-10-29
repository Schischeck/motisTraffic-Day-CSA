#include "motis/reliability/tools/flatbuffers_tools.h"

#include <string>
#include <tuple>
#include <vector>

#include "motis/core/common/date_util.h"
#include "motis/core/common/journey_builder.h"
#include "motis/core/schedule/time.h"

#include "motis/routing/response_builder.h"

#include "motis/reliability/probability_distribution.h"
#include "motis/reliability/rating/connection_rating.h"
#include "motis/reliability/rating/simple_rating.h"

using namespace flatbuffers;

namespace motis {
namespace reliability {
namespace flatbuffers_tools {

time_t to_unix_time(std::tuple<int, int, int> ddmmyyyy, motis::time time) {
  return motis_to_unixtime(
      motis::to_unix_time(std::get<2>(ddmmyyyy), std::get<1>(ddmmyyyy),
                          std::get<0>(ddmmyyyy)),
      time);
}

module::msg_ptr to_flatbuffers_message(routing::RoutingRequest const* request) {
  /* convert routing::RoutingRequest to Offset<RoutingRequest> */
  FlatBufferBuilder b;
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
  b.Finish(CreateMessage(
      b, MsgContent_RoutingRequest,
      routing::CreateRoutingRequest(b, &interval, request->type(),
                                    request->direction(),
                                    b.CreateVector(station_elements)).Union()));
  return module::make_msg(b);
}

module::msg_ptr to_routing_request(std::string const& from_name,
                                   std::string const& from_eva,
                                   std::string const& to_name,
                                   std::string const& to_eva,
                                   time_t interval_begin, time_t interval_end) {
  FlatBufferBuilder b;
  std::vector<Offset<routing::StationPathElement>> station_elements;
  station_elements.push_back(routing::CreateStationPathElement(
      b, b.CreateString(from_name), b.CreateString(from_eva)));
  station_elements.push_back(routing::CreateStationPathElement(
      b, b.CreateString(to_name), b.CreateString(to_eva)));
  routing::Interval interval(interval_begin, interval_end);
  b.Finish(CreateMessage(
      b, MsgContent_RoutingRequest,
      routing::CreateRoutingRequest(b, &interval, routing::Type::Type_PreTrip,
                                    routing::Direction::Direction_Forward,
                                    b.CreateVector(station_elements)).Union()));
  return module::make_msg(b);
}

module::msg_ptr to_routing_request(std::string const& from_name,
                                   std::string const& from_eva,
                                   std::string const& to_name,
                                   std::string const& to_eva,
                                   motis::time interval_begin,
                                   motis::time interval_end,
                                   std::tuple<int, int, int> ddmmyyyy) {
  return to_routing_request(from_name, from_eva, to_name, to_eva,
                            to_unix_time(ddmmyyyy, interval_begin),
                            to_unix_time(ddmmyyyy, interval_end));
}

Offset<routing::RoutingResponse> convert_routing_response(
    FlatBufferBuilder& b, routing::RoutingResponse const* orig_routing_response,
    std::vector<std::unique_ptr<category>> const& categories) {
  using namespace routing;
  std::vector<Offset<routing::Connection>> connections;
  auto journeys =
      journey_builder::to_journeys(orig_routing_response, categories);
  for (auto const& j : journeys) {
    connections.push_back(CreateConnection(
        b, b.CreateVector(detail::convert_stops(b, j.stops)),
        b.CreateVector(detail::convert_moves(b, j.transports)),
        b.CreateVector(detail::convert_attributes(b, j.attributes))));
  }
  return CreateRoutingResponse(b, b.CreateVector(connections));
}

namespace rating_converter {

Offset<reliability::ProbabilityDistribution> convert(
    FlatBufferBuilder& b, probability_distribution const& pd,
    time_t event_time) {
  std::vector<float> probabilities;
  pd.get_probabilities(probabilities);
  return CreateProbabilityDistribution(b, event_time + (pd.first_minute() * 60),
                                       b.CreateVector(probabilities),
                                       (float)pd.sum());
}

/* write the distributions for all events */
std::vector<Offset<RatingElement>> convert_rating_elements(
    FlatBufferBuilder& b, rating::connection_rating const& conn_rating,
    routing::Connection const* orig_conn) {
  std::vector<Offset<RatingElement>> rating_elements;
  for (auto e : conn_rating.public_transport_ratings_) {
    Range r(e.departure_stop_idx_, e.arrival_stop_idx());
    rating_elements.push_back(CreateRatingElement(
        b, &r,
        convert(
            b, e.departure_distribution_,
            (*orig_conn->stops())[e.departure_stop_idx_]->departure()->time()),
        convert(
            b, e.arrival_distribution_,
            (*orig_conn->stops())[e.arrival_stop_idx()]->arrival()->time())));
  }
  return rating_elements;
}

/* for each transport, write the distribution of
 * the first departure and the last arrival */
std::vector<Offset<RatingElement>> convert_rating_elements_short(
    FlatBufferBuilder& b, rating::connection_rating const& conn_rating,
    routing::Connection const* orig_conn) {
  std::vector<Offset<RatingElement>> rating_elements;
  for (auto it_t = orig_conn->transports()->begin();
       it_t != orig_conn->transports()->end(); ++it_t) {
    if (it_t->move_type() == routing::Move_Transport) {
      auto transport = (routing::Transport const*)it_t->move();
      auto const rating_from = std::find_if(
          conn_rating.public_transport_ratings_.begin(),
          conn_rating.public_transport_ratings_.end(),
          [transport](rating::rating_element const& rating) {
            return transport->range()->from() == rating.departure_stop_idx_;
          });
      auto const rating_to = std::find_if(
          conn_rating.public_transport_ratings_.begin(),
          conn_rating.public_transport_ratings_.end(),
          [transport](rating::rating_element const& rating) {
            return transport->range()->to() == rating.arrival_stop_idx();
          });
      if (rating_from == conn_rating.public_transport_ratings_.end() ||
          rating_to == conn_rating.public_transport_ratings_.end()) {
        std::cout << "\nWarning(convert_rating_elements_short) failure."
                  << std::endl;
        break;
      }
      Range r(rating_from->departure_stop_idx_, rating_to->arrival_stop_idx());
      rating_elements.push_back(CreateRatingElement(
          b, &r, convert(b, rating_from->departure_distribution_,
                         (*orig_conn->stops())[rating_from->departure_stop_idx_]
                             ->departure()
                             ->time()),
          convert(b, rating_to->arrival_distribution_,
                  (*orig_conn->stops())[rating_to->arrival_stop_idx()]
                      ->arrival()
                      ->time())));
    }
  }
  return rating_elements;
}

Offset<Vector<Offset<Rating>>> convert_ratings(
    FlatBufferBuilder& b,
    std::vector<rating::connection_rating> const& orig_ratings,
    Vector<Offset<routing::Connection>> const& orig_connections,
    bool const short_output) {
  std::vector<Offset<Rating>> v_conn_ratings;
  for (unsigned int c_idx = 0; c_idx < orig_ratings.size(); ++c_idx) {
    auto const& conn_rating = orig_ratings[c_idx];
    auto const orig_conn = orig_connections[c_idx];
    v_conn_ratings.push_back(CreateRating(
        b, b.CreateVector(
               short_output
                   ? convert_rating_elements_short(b, conn_rating, orig_conn)
                   : convert_rating_elements(b, conn_rating, orig_conn)),
        (float)conn_rating.connection_rating_));
  }
  return b.CreateVector(v_conn_ratings);
}
}  // namespace rating_converter

namespace simple_rating_converter {
std::vector<Offset<SimpleRatingElement>> convert_simple_rating_elements(
    FlatBufferBuilder& b,
    rating::simple_rating::simple_connection_rating const& conn_rating,
    routing::Connection const* orig_conn) {
  std::vector<Offset<SimpleRatingElement>> rating_elements;
  for (auto e : conn_rating.ratings_elements_) {
    Range r(e.from_, e.to_);
    std::vector<Offset<SimpleRatingInfo>> rating_infos;
    for (auto const& info : e.ratings_) {
      rating_infos.push_back(CreateSimpleRatingInfo(
          b, b.CreateString(rating::simple_rating::to_string(info.first)),
          info.second));
    }
    rating_elements.push_back(
        CreateSimpleRatingElement(b, &r, b.CreateVector(rating_infos)));
  }
  return rating_elements;
}

Offset<Vector<Offset<SimpleRating>>> convert_simple_ratings(
    FlatBufferBuilder& b,
    std::vector<rating::simple_rating::simple_connection_rating> const&
        orig_ratings,
    Vector<Offset<routing::Connection>> const& orig_connections) {
  std::vector<Offset<SimpleRating>> v_conn_ratings;
  for (unsigned int c_idx = 0; c_idx < orig_ratings.size(); ++c_idx) {
    auto const& conn_rating = orig_ratings[c_idx];
    auto const orig_conn = orig_connections[c_idx];
    v_conn_ratings.push_back(
        CreateSimpleRating(b, b.CreateVector(convert_simple_rating_elements(
                                  b, conn_rating, orig_conn)),
                           (float)conn_rating.connection_rating_));
  }
  return b.CreateVector(v_conn_ratings);
}
}  // namespace simple_rating_converter

module::msg_ptr to_reliable_routing_response(
    routing::RoutingResponse const* orig_routing_response,
    std::vector<std::unique_ptr<category>> const& categories,
    std::vector<rating::connection_rating> const& orig_ratings,
    std::vector<rating::simple_rating::simple_connection_rating> const&
        orig_simple_ratings,
    bool const short_output) {
  assert(orig_routing_response->connections()->size() == orig_ratings.size());
  FlatBufferBuilder b;
  auto const routing_response =
      convert_routing_response(b, orig_routing_response, categories);
  auto const conn_ratings = rating_converter::convert_ratings(
      b, orig_ratings, *orig_routing_response->connections(), short_output);
  auto const simple_ratings = simple_rating_converter::convert_simple_ratings(
      b, orig_simple_ratings, *orig_routing_response->connections());
  b.Finish(CreateMessage(
      b, MsgContent_ReliableRoutingResponse,
      CreateReliableRoutingResponse(b, routing_response, conn_ratings,
                                    simple_ratings).Union()));
  return module::make_msg(b);
}

module::msg_ptr to_reliable_routing_request(
    std::string const& from_name, std::string const& from_eva,
    std::string const& to_name, std::string const& to_eva,
    motis::time interval_begin, motis::time interval_end,
    std::tuple<int, int, int> ddmmyyyy, RequestType type) {
  FlatBufferBuilder b;
  std::vector<Offset<routing::StationPathElement>> station_elements;
  station_elements.push_back(routing::CreateStationPathElement(
      b, b.CreateString(from_name), b.CreateString(from_eva)));
  station_elements.push_back(routing::CreateStationPathElement(
      b, b.CreateString(to_name), b.CreateString(to_eva)));
  routing::Interval interval(to_unix_time(ddmmyyyy, interval_begin),
                             to_unix_time(ddmmyyyy, interval_end));
  b.Finish(CreateMessage(b, MsgContent_ReliableRoutingRequest,
                         reliability::CreateReliableRoutingRequest(
                             b, routing::CreateRoutingRequest(
                                    b, &interval, routing::Type::Type_PreTrip,
                                    routing::Direction::Direction_Forward,
                                    b.CreateVector(station_elements)),
                             type).Union()));
  return module::make_msg(b);
}

}  // namespace flatbuffers_tools
}  // namespace reliability
}  // namespace motis
