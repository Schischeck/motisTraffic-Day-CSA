#include "motis/reliability/tools/flatbuffers/response_builder.h"

#include <string>
#include <tuple>
#include <vector>

#include "motis/core/schedule/time.h"
#include "motis/core/journey/journey.h"
#include "motis/core/journey/journeys_to_message.h"
#include "motis/core/journey/message_to_journeys.h"

#include "motis/reliability/distributions/probability_distribution.h"
#include "motis/reliability/rating/cg_arrival_distribution.h"
#include "motis/reliability/rating/connection_rating.h"
#include "motis/reliability/rating/simple_rating.h"
#include "motis/reliability/search/connection_graph.h"

using namespace flatbuffers;

namespace motis {
namespace reliability {
namespace flatbuffers {
namespace response_builder {

namespace rating_converter {

Offset<reliability::ProbabilityDistribution> convert(
    FlatBufferBuilder& b, probability_distribution const& pd,
    time_t scheduled_event_time) {
  std::vector<float> probabilities;
  pd.get_probabilities(probabilities);
  auto fpd = CreateProbabilityDistribution(
      b, scheduled_event_time + (pd.first_minute() * 60),
      b.CreateVector(probabilities), static_cast<float>(pd.sum()));
  return fpd;
}

/* write the distributions for all events */
std::vector<Offset<RatingElement>> convert_rating_elements(
    FlatBufferBuilder& b, rating::connection_rating const& conn_rating,
    journey const& orig_conn) {
  std::vector<Offset<RatingElement>> rating_elements;
  for (auto e : conn_rating.public_transport_ratings_) {
    Range r(e.departure_stop_idx_, e.arrival_stop_idx());
    rating_elements.push_back(CreateRatingElement(
        b, &r, convert(b, e.departure_distribution_,
                       orig_conn.stops_[e.departure_stop_idx_]
                           .departure_.schedule_timestamp_),
        convert(b, e.arrival_distribution_,
                orig_conn.stops_[e.arrival_stop_idx()]
                    .arrival_.schedule_timestamp_)));
  }
  return rating_elements;
}

/* for each transport, write the distribution of
 * the first departure and the last arrival */
std::vector<Offset<RatingElement>> convert_rating_elements_short(
    FlatBufferBuilder& b, rating::connection_rating const& conn_rating,
    journey const& orig_conn) {
  std::vector<Offset<RatingElement>> rating_elements;
  for (auto const& trans : orig_conn.transports_) {
    if (trans.is_walk_) {
      continue;
    }
    auto const rating_from =
        std::find_if(conn_rating.public_transport_ratings_.begin(),
                     conn_rating.public_transport_ratings_.end(),
                     [trans](rating::rating_element const& rating) {
                       return trans.from_ == rating.departure_stop_idx_;
                     });
    auto const rating_to =
        std::find_if(conn_rating.public_transport_ratings_.begin(),
                     conn_rating.public_transport_ratings_.end(),
                     [trans](rating::rating_element const& rating) {
                       return trans.to_ == rating.arrival_stop_idx();
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
                       orig_conn.stops_[rating_from->departure_stop_idx_]
                           .departure_.schedule_timestamp_),
        convert(b, rating_to->arrival_distribution_,
                orig_conn.stops_[rating_to->arrival_stop_idx()]
                    .arrival_.schedule_timestamp_)));
  }
  return rating_elements;
}

Offset<Vector<Offset<Rating>>> convert_ratings(
    FlatBufferBuilder& b,
    std::vector<rating::connection_rating> const& orig_ratings,
    std::vector<journey> const& orig_connections, bool const short_output) {
  std::vector<Offset<Rating>> v_conn_ratings;
  for (unsigned c_idx = 0; c_idx < orig_ratings.size(); ++c_idx) {
    auto const& conn_rating = orig_ratings[c_idx];
    auto const orig_conn = orig_connections[c_idx];
    v_conn_ratings.push_back(CreateRating(
        b, b.CreateVector(
               short_output
                   ? convert_rating_elements_short(b, conn_rating, orig_conn)
                   : convert_rating_elements(b, conn_rating, orig_conn)),
        static_cast<float>(conn_rating.connection_rating_)));
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
  for (auto const& conn_rating : orig_ratings) {
    v_conn_ratings.push_back(CreateSimpleRating(
        b, b.CreateVector(convert_simple_rating_elements(b, conn_rating)),
        static_cast<float>(conn_rating.connection_rating_)));
  }
  return b.CreateVector(v_conn_ratings);
}
}  // namespace simple_rating_converter

Offset<routing::RoutingResponse> to_routing_response(
    FlatBufferBuilder& b, std::vector<journey> const& journeys) {
  std::vector<Offset<Connection>> connections;
  for (auto& j : journeys) {
    connections.push_back(to_connection(b, j));
  }
  return routing::CreateRoutingResponse(b, 0, b.CreateVector(connections));
}

module::msg_ptr to_reliability_rating_response(
    std::vector<journey> const& journeys,
    std::vector<rating::connection_rating> const& orig_ratings,
    std::vector<rating::simple_rating::simple_connection_rating> const&
        orig_simple_ratings,
    bool const short_output) {
  assert(journeys.size() == orig_ratings.size());
  module::message_creator b;
  b.ForceDefaults(true); /* necessary to write indices 0 */
  auto const routing_response = to_routing_response(b, journeys);
  auto const conn_ratings = rating_converter::convert_ratings(
      b, orig_ratings, journeys, short_output);
  auto const simple_ratings =
      simple_rating_converter::convert_simple_ratings(b, orig_simple_ratings);
  b.create_and_finish(MsgContent_ReliabilityRatingResponse,
                      CreateReliabilityRatingResponse(
                          b, routing_response, conn_ratings, simple_ratings)
                          .Union());
  return module::make_msg(b);
}

std::pair<std::time_t, std::time_t> get_scheduled_times(
    journey const& journey) {
  auto const& first_transport =
      std::find_if(journey.transports_.begin(), journey.transports_.end(),
                   [&](journey::transport const& t) { return !t.is_walk_; });
  auto const& last_transport =
      std::find_if(journey.transports_.rbegin(), journey.transports_.rend(),
                   [&](journey::transport const& t) { return !t.is_walk_; });
  auto const scheduled_departure =
      first_transport != journey.transports_.end()
          ? journey.stops_[first_transport->from_]
                .departure_.schedule_timestamp_
          : journey.stops_.front()
                .departure_
                .schedule_timestamp_ /* transport consists of a walk */;
  auto const scheduled_arrival =
      last_transport != journey.transports_.rend()
          ? journey.stops_[last_transport->to_].arrival_.schedule_timestamp_
          : journey.stops_.back()
                .arrival_
                .schedule_timestamp_ /* transport consists of a walk */;

  return std::make_pair(scheduled_departure, scheduled_arrival);
}

Offset<ConnectionGraph> to_connection_graph(
    FlatBufferBuilder& b, search::connection_graph const& cg) {
  std::vector<Offset<Stop>> stops;
  for (auto const& stop : cg.stops_) {
    std::vector<Offset<Alternative>> alternative_infos;
    for (auto const& alternative_info : stop.alternative_infos_) {
      auto const& journey = cg.journeys_.at(alternative_info.journey_index_);

      auto const scheduled_times = get_scheduled_times(journey);
      auto dep_dist = rating_converter::convert(
          b, alternative_info.rating_.departure_distribution_,
          scheduled_times.first);
      auto arr_dist = rating_converter::convert(
          b, alternative_info.rating_.arrival_distribution_,
          scheduled_times.second);
      auto rating = CreateAlternativeRating(b, dep_dist, arr_dist);

      alternative_infos.push_back(
          CreateAlternative(b, alternative_info.journey_index_,
                            alternative_info.next_stop_index_, rating));
    }
    stops.push_back(
        CreateStop(b, stop.index_, b.CreateVector(alternative_infos)));
  }

  std::vector<Offset<Connection>> journeys;
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
  module::message_creator b;
  b.ForceDefaults(true); /* necessary to write indices 0 */
  std::vector<Offset<ConnectionGraph>> connection_graphs;
  for (auto const cg : cgs) {
    connection_graphs.push_back(to_connection_graph(b, *cg));
  }
  b.create_and_finish(MsgContent_ReliableRoutingResponse,
                      reliability::CreateReliableRoutingResponse(
                          b, b.CreateVector(connection_graphs))
                          .Union());
  return module::make_msg(b);
}

}  // namespace response_builder
}  // namespace flatbuffers
}  // namespace reliability
}  // namespace motis
