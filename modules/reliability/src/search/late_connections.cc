#include "motis/reliability/search/late_connections.h"

#include <algorithm>

#include "motis/core/common/constants.h"
#include "motis/core/common/geo.h"
#include "motis/core/common/logging.h"
#include "motis/core/schedule/schedule.h"
#include "motis/core/access/station_access.h"
#include "motis/core/journey/message_to_journeys.h"

#include "motis/module/context/motis_call.h"
#include "motis/module/message.h"

#include "motis/protocol/LookupGeoStationIdRequest_generated.h"
#include "motis/protocol/LookupGeoStationResponse_generated.h"

#include "motis/reliability/intermodal/hotels.h"
#include "motis/reliability/intermodal/individual_modes_container.h"
#include "motis/reliability/rating/reliability_rating.h"
#include "motis/reliability/reliability.h"
#include "motis/reliability/tools/flatbuffers/request_builder.h"
#include "motis/reliability/tools/flatbuffers/response_builder.h"

namespace motis {
namespace reliability {
namespace search {
namespace late_connections {
namespace detail {

constexpr auto TAXI_BASE_PRICE = 250;
constexpr auto TAXI_KM_PRICE = 200;
constexpr auto TAXI_BASE_TIME = 10;  // in minutes (entering and leaving time)
constexpr auto TAXI_AVG_SPEED_SHORT_DISTANCE = 40;  // km/h
constexpr auto TAXI_AVG_SPEED_LONG_DISTANCE = 100;  // km/h

constexpr auto M_PER_KM = 1000.0;
constexpr auto MIN_PER_HOUR = 60;
constexpr auto DISTANCE_THRESHOLD = 5.0;  // 5km
constexpr auto AIR_DISTANCE_CORRECTION_FACTOR_CITY = 1.5;
constexpr auto AIR_DISTANCE_CORRECTION_FACTOR_HIGHWAY = 1.2;

taxi_cost::taxi_cost(double const& lat1, double const& lon1, double const& lat2,
                     double const& lon2, unsigned const taxi_base_price,
                     unsigned const taxi_km_price,
                     unsigned const taxi_base_time,
                     unsigned const taxi_avg_speed_short_distance,
                     unsigned const taxi_avg_speed_long_distance) {
  auto const distance_in_km =
      (geo_detail::distance_in_m(lat1, lon1, lat2, lon2) / M_PER_KM);

  auto const city_distance = std::min(distance_in_km, DISTANCE_THRESHOLD) *
                             AIR_DISTANCE_CORRECTION_FACTOR_CITY;
  auto const highway_distance =
      std::max(distance_in_km - DISTANCE_THRESHOLD, 0.0) *
      AIR_DISTANCE_CORRECTION_FACTOR_HIGHWAY;

  duration_ = taxi_base_time +
              ((city_distance / taxi_avg_speed_short_distance) +
               (highway_distance / taxi_avg_speed_long_distance)) *
                  MIN_PER_HOUR;

  price_ = taxi_base_price + (city_distance + highway_distance) * taxi_km_price;
}

void ask_lookup_module(
    station const& destination, unsigned const taxi_radius,
    std::vector<intermodal::individual_modes_container::taxi>& taxis) {
  using namespace lookup;
  module::message_creator b;
  b.create_and_finish(
      MsgContent_LookupGeoStationIdRequest,
      CreateLookupGeoStationIdRequest(b, b.CreateString(destination.eva_nr_),
                                      static_cast<double>(taxi_radius))
          .Union(),
      "/lookup/geo_station_id");
  auto res_msg = motis_call(module::make_msg(b))->val();
  auto lookup_res = motis_content(LookupGeoStationResponse, res_msg);
  for (auto const& st : *lookup_res->stations()) {
    // no taxis from local stations
    // in order to obtain better search times and reduce
    // the number of pareto-optimal connections
    if (st->id()->str().substr(0, 2) != "80") {
      continue;
    }
    taxi_cost cost(st->pos()->lat(), st->pos()->lng(), destination.lat(),
                   destination.lng(), TAXI_BASE_PRICE, TAXI_KM_PRICE,
                   TAXI_BASE_TIME, TAXI_AVG_SPEED_SHORT_DISTANCE,
                   TAXI_AVG_SPEED_LONG_DISTANCE);
    taxis.emplace_back(st->id()->str(), destination.eva_nr_, cost.duration_,
                       cost.price_);
  }
}

station const& get_destination(ReliableRoutingRequest const& req,
                               schedule const& sched) {
  auto destination_eva = req.request()->destination()->id()->str();
  if (destination_eva.empty()) {
    module::message_creator b;
    b.create_and_finish(
        MsgContent_StationGuesserRequest,
        guesser::CreateStationGuesserRequest(
            b, 1, b.CreateString(req.request()->destination()->name()->str()))
            .Union(),
        "/guesser");
    auto const msg = motis_call(make_msg(b))->val();
    using guesser::StationGuesserResponse;
    auto const guesses = motis_content(StationGuesserResponse, msg)->guesses();
    if (guesses->size() == 0) {
      throw std::system_error(error::failure);
    }
    destination_eva = guesses->Get(0)->id()->str();
  }
  return *get_station(sched, destination_eva);
}

void init_taxis(
    ReliableRoutingRequest const& req, schedule const& sched,
    std::vector<intermodal::individual_modes_container::taxi>& taxis) {
  if (req.request_type()->request_options_type() !=
      RequestOptions_LateConnectionReq) {
    throw std::system_error(error::failure);
  }
  auto ops = reinterpret_cast<LateConnectionReq const*>(
      req.request_type()->request_options());

  auto const& destination = get_destination(req, sched);

  ask_lookup_module(destination, ops->taxi_radius(), taxis);
}

void init_hotels(ReliableRoutingRequest const& req, schedule const& sched,
                 std::string const& hotels_file,
                 std::vector<intermodal::hotel>& hotels) {
  if (req.request_type()->request_options_type() !=
      RequestOptions_LateConnectionReq) {
    throw std::system_error(error::failure);
  }
  auto ops = reinterpret_cast<LateConnectionReq const*>(
      req.request_type()->request_options());

  std::vector<intermodal::hotel> tmp;
  parse_hotels(hotels_file, tmp, ops->hotel_earliest_checkout(),
               ops->hotel_min_stay(), ops->hotel_price());
  for (auto const& h : tmp) {
    auto it = sched.eva_to_station_.find(h.station_);
    if (it != end(sched.eva_to_station_)) {
      hotels.push_back(h);
    } else {
      LOG(logging::warn) << "Could not find hotel-station " << h.station_;
    }
  }
}

module::msg_ptr ask_routing(ReliableRoutingRequest const& req,
                            std::string const& hotels_file,
                            schedule const& sched) {
  using namespace motis::reliability::intermodal;
  individual_modes_container container;
  detail::init_hotels(req, sched, hotels_file, container.hotels_);
  detail::init_taxis(req, sched, container.taxis_);
  flatbuffers::request_builder b(req);
  b.add_additional_edges(container);
  return motis_call(b.build_routing_request())->val();
}

unsigned estimate_price(journey const& j) {
  constexpr unsigned MAX_PRICE = 12300;
  constexpr double KM_PRICE = 30.0;
  auto class_factor = [](unsigned const train_class) -> double {
    switch (train_class) {
      case 4: return 1.5;
      case 3: return 1.2;
      case 2: return 1.1;
      default: return 1.0;
    }
  };
  if (j.stops_.size() < 2 || j.transports_.empty()) {
    throw std::system_error(error::failure);
  }
  auto const distance_in_km =
      (geo_detail::distance_in_m(j.stops_.front().lat_, j.stops_.front().lng_,
                                 j.stops_.back().lat_, j.stops_.back().lng_) /
       M_PER_KM);
  auto const highest_class = std::max_element(
      j.transports_.begin(), j.transports_.end(),
      [](journey::transport const& t1, journey::transport const& t2) {
        return t1.clasz_ < t2.clasz_;
      });

  auto const price = static_cast<unsigned>(
      distance_in_km * class_factor(highest_class->clasz_) * KM_PRICE);
  return std::min(MAX_PRICE, price);
}

unsigned calc_compensation(journey const& orig_journey,
                           journey const& alternative) {
  auto compensation_factor = [](int const delay) -> double {
    if (delay >= 120) {
      return 0.5;
    } else if (delay >= 60) {
      return 0.25;
    }
    return 0.0;
  };

  auto const delay = (alternative.stops_.back().arrival_.timestamp_ -
                      orig_journey.stops_.back().arrival_.timestamp_) /
                     60;
  return static_cast<unsigned>(compensation_factor(delay) *
                               orig_journey.price_);
}

void update_db_costs(std::vector<journey>& journeys, journey orig_conn) {
  auto no_hotel_or_taxi = [](journey const& j) {
    return std::find_if(
               j.transports_.begin(), j.transports_.end(), [](auto const& t) {
                 return t.is_walk_ &&
                        (!t.mumo_type_.empty() &&
                         t.mumo_type_ != intermodal::to_str(intermodal::WALK));
               }) == j.transports_.end();
  };
  orig_conn.price_ = estimate_price(orig_conn);
  for (auto& j : journeys) {
    if (no_hotel_or_taxi(j)) {
      j.db_costs_ = calc_compensation(orig_conn, j);
    }
  }
}

void update_db_costs(std::vector<journey>& journeys,
                     ReliableRoutingRequest const& req) {
  if (req.request_type()->request_options_type() !=
      RequestOptions_LateConnectionReq) {
    throw std::system_error(error::failure);
  }
  auto ops = reinterpret_cast<LateConnectionReq const*>(
      req.request_type()->request_options());
  update_db_costs(journeys, convert(ops->original_connection()));
}

}  // namespace detail

module::msg_ptr search(ReliableRoutingRequest const& req, reliability& rel,
                       std::string const& hotels_file) {
  auto lock = rel.synced_sched();
  auto routing_res = detail::ask_routing(req, hotels_file, lock.sched());
  using routing::RoutingResponse;
  auto journeys =
      message_to_journeys(motis_content(RoutingResponse, routing_res));
  auto ratings = rating::rate_journeys(
      journeys,
      motis::reliability::context(lock.sched(), rel.precomputed_distributions(),
                                  rel.s_t_distributions()));

  for (auto& j : journeys) {
    intermodal::update_mumo_info(j);
  }

  detail::update_db_costs(journeys, req);

  return flatbuffers::response_builder::to_reliability_rating_response(
      journeys, ratings.first, ratings.second, true /* short output */);
}

}  // namespace late_connections
}  // namespace search
}  // namespace reliability
}  // namespace motis
