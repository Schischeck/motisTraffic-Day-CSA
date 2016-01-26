#include "motis/reliability/tools/flatbuffers_tools.h"

#include <string>
#include <tuple>
#include <vector>

#include "motis/core/common/date_util.h"
#include "motis/core/journey/journeys_to_message.h"
#include "motis/core/journey/message_to_journeys.h"
#include "motis/core/schedule/time.h"

#include "motis/reliability/probability_distribution.h"
#include "motis/reliability/rating/cg_arrival_distribution.h"
#include "motis/reliability/rating/connection_rating.h"
#include "motis/reliability/rating/simple_rating.h"
#include "motis/reliability/search/connection_graph.h"

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
  b.CreateAndFinish(MsgContent_RoutingRequest,
                    routing::CreateRoutingRequest(
                        b, &interval, request->type(), request->direction(),
                        b.CreateVector(station_elements))
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
  b.CreateAndFinish(MsgContent_RoutingRequest,
                    routing::CreateRoutingRequest(
                        b, &interval, (ontrip ? routing::Type::Type_OnTrip
                                              : routing::Type::Type_PreTrip),
                        routing::Direction::Direction_Forward,
                        b.CreateVector(station_elements))
                        .Union());
  return module::make_msg(b);
}

module::msg_ptr to_routing_request(
    std::string const& from_name, std::string const& from_eva,
    std::string const& to_name, std::string const& to_eva,
    motis::time interval_begin, motis::time interval_end,
    std::tuple<int, int, int> ddmmyyyy, bool const ontrip) {
  return to_routing_request(from_name, from_eva, to_name, to_eva,
                            to_unix_time(ddmmyyyy, interval_begin),
                            to_unix_time(ddmmyyyy, interval_end), ontrip);
}

Offset<routing::RoutingResponse> convert_routing_response(
    FlatBufferBuilder& b,
    routing::RoutingResponse const* orig_routing_response) {
  std::vector<Offset<routing::Connection>> connections;
  auto const journeys = message_to_journeys(orig_routing_response);
  for (auto const& j : journeys) {
    connections.push_back(to_connection(b, j));
  }
  return CreateRoutingResponse(b, 0, b.CreateVector(connections));
}

namespace rating_converter {

Offset<reliability::ProbabilityDistribution> convert(
    FlatBufferBuilder& b, probability_distribution const& pd,
    time_t event_time) {
  std::vector<float> probabilities;
  pd.get_probabilities(probabilities);
  auto fpd = CreateProbabilityDistribution(
      b, event_time + (pd.first_minute() * 60), b.CreateVector(probabilities),
      (float)pd.sum());
  return fpd;
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
      auto const rating_from =
          std::find_if(conn_rating.public_transport_ratings_.begin(),
                       conn_rating.public_transport_ratings_.end(),
                       [transport](rating::rating_element const& rating) {
                         return (unsigned int)transport->range()->from() ==
                                rating.departure_stop_idx_;
                       });
      auto const rating_to =
          std::find_if(conn_rating.public_transport_ratings_.begin(),
                       conn_rating.public_transport_ratings_.end(),
                       [transport](rating::rating_element const& rating) {
                         return (unsigned int)transport->range()->to() ==
                                rating.arrival_stop_idx();
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
    rating::simple_rating::simple_connection_rating const& conn_rating) {
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
        orig_ratings) {
  std::vector<Offset<SimpleRating>> v_conn_ratings;
  for (unsigned int c_idx = 0; c_idx < orig_ratings.size(); ++c_idx) {
    auto const& conn_rating = orig_ratings[c_idx];
    v_conn_ratings.push_back(CreateSimpleRating(
        b, b.CreateVector(convert_simple_rating_elements(b, conn_rating)),
        (float)conn_rating.connection_rating_));
  }
  return b.CreateVector(v_conn_ratings);
}
}  // namespace simple_rating_converter

module::msg_ptr to_reliability_rating_response(
    routing::RoutingResponse const* orig_routing_response,
    std::vector<rating::connection_rating> const& orig_ratings,
    std::vector<rating::simple_rating::simple_connection_rating> const&
        orig_simple_ratings,
    bool const short_output) {
  assert(orig_routing_response->connections()->size() == orig_ratings.size());
  module::MessageCreator b;
  b.ForceDefaults(true); /* necessary to write indices 0 */
  auto const routing_response =
      convert_routing_response(b, orig_routing_response);
  auto const conn_ratings = rating_converter::convert_ratings(
      b, orig_ratings, *orig_routing_response->connections(), short_output);
  auto const simple_ratings =
      simple_rating_converter::convert_simple_ratings(b, orig_simple_ratings);
  b.CreateAndFinish(MsgContent_ReliabilityRatingResponse,
                    CreateReliabilityRatingResponse(
                        b, routing_response, conn_ratings, simple_ratings)
                        .Union());
  return module::make_msg(b);
}

module::msg_ptr to_reliable_routing_request(
    module::MessageCreator& b, std::string const& from_name,
    std::string const& from_eva, std::string const& to_name,
    std::string const& to_eva, motis::time interval_begin,
    motis::time interval_end, std::tuple<int, int, int> ddmmyyyy,
    Offset<RequestOptionsWrapper>& request_type_wrapper) {
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
                               b.CreateVector(station_elements)),
                        request_type_wrapper)
                        .Union());
  return module::make_msg(b);
}

module::msg_ptr to_rating_request(std::string const& from_name,
                                  std::string const& from_eva,
                                  std::string const& to_name,
                                  std::string const& to_eva,
                                  motis::time interval_begin,
                                  motis::time interval_end,
                                  std::tuple<int, int, int> ddmmyyyy) {
  module::MessageCreator b;
  auto request_type_wrapper = reliability::CreateRequestOptionsWrapper(
      b, reliability::RequestOptions_RatingReq,
      reliability::CreateRatingReq(b).Union());
  return to_reliable_routing_request(b, from_name, from_eva, to_name, to_eva,
                                     interval_begin, interval_end, ddmmyyyy,
                                     request_type_wrapper);
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
  return to_reliable_routing_request(b, from_name, from_eva, to_name, to_eva,
                                     interval_begin, interval_end, ddmmyyyy,
                                     request_type_wrapper);
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
  return to_reliable_routing_request(b, from_name, from_eva, to_name, to_eva,
                                     interval_begin, interval_end, ddmmyyyy,
                                     request_type_wrapper);
}

Offset<ConnectionGraph> to_connection_graph(
    FlatBufferBuilder& b, search::connection_graph const& cg) {
  std::vector<Offset<Stop>> stops;

  for (auto const& stop : cg.stops_) {
    std::vector<Offset<Alternative>> alternative_infos;
    for (auto const& alternative_info : stop.alternative_infos_) {
      auto const& journey = cg.journeys_.at(alternative_info.journey_index_);
      auto dep_dist = rating_converter::convert(
          b, alternative_info.rating_.departure_distribution_,
          journey.stops.front().departure.timestamp);
      auto arr_dist = rating_converter::convert(
          b, alternative_info.rating_.arrival_distribution_,
          journey.stops.back().arrival.timestamp);
      auto rating = CreateAlternativeRating(b, dep_dist, arr_dist);

      alternative_infos.push_back(
          CreateAlternative(b, alternative_info.journey_index_,
                            alternative_info.next_stop_index_, rating));
    }
    stops.push_back(
        CreateStop(b, stop.index_, b.CreateVector(alternative_infos)));
  }

  std::vector<Offset<routing::Connection>> journeys;
  for (auto const& j : cg.journeys_) {
    journeys.push_back(to_connection(b, j));
  }

  auto const arr_dist = rating::cg::calc_arrival_distribution(cg);
  return CreateConnectionGraph(
      b, b.CreateVector(stops), b.CreateVector(journeys),
      rating_converter::convert(b, arr_dist.second, arr_dist.first));
}

module::msg_ptr to_reliable_routing_response(
    std::vector<std::shared_ptr<search::connection_graph>> const& cgs) {
  module::MessageCreator b;
  b.ForceDefaults(true); /* necessary to write indices 0 */
  std::vector<Offset<ConnectionGraph>> connection_graphs;
  for (auto const cg : cgs) {
    connection_graphs.push_back(to_connection_graph(b, *cg));
  }
  b.CreateAndFinish(MsgContent_ReliableRoutingResponse,
                    reliability::CreateReliableRoutingResponse(
                        b, b.CreateVector(connection_graphs))
                        .Union());
  return module::make_msg(b);
}

}  // namespace flatbuffers_tools
}  // namespace reliability
}  // namespace motis
