#include "motis/reliability/search/late_connections.h"

#include <algorithm>

#include "motis/core/common/constants.h"
#include "motis/core/common/geo.h"
#include "motis/core/schedule/schedule.h"
#include "motis/core/access/error.h"

#include "motis/module/context/motis_call.h"
#include "motis/module/message.h"

#include "motis/protocol/LookupGeoStationIdRequest_generated.h"
#include "motis/protocol/LookupGeoStationResponse_generated.h"

#include "motis/reliability/intermodal/hotels.h"
#include "motis/reliability/intermodal/individual_modes_container.h"
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
    std::string const& destination, double const& destination_lat,
    double const& destination_lng, unsigned const taxi_radius,
    std::vector<intermodal::individual_modes_container::taxi>& taxis) {
  using namespace lookup;
  module::message_creator b;
  b.create_and_finish(
      MsgContent_LookupGeoStationIdRequest,
      CreateLookupGeoStationIdRequest(b, b.CreateString(destination),
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
    taxi_cost cost(st->pos()->lat(), st->pos()->lng(), destination_lat,
                   destination_lng, TAXI_BASE_PRICE, TAXI_KM_PRICE,
                   TAXI_BASE_TIME, TAXI_AVG_SPEED_SHORT_DISTANCE,
                   TAXI_AVG_SPEED_LONG_DISTANCE);
    taxis.emplace_back(st->id()->str(), destination, cost.duration_,
                       cost.price_);
  }
}

void init_taxi(
    ReliableRoutingRequest const& req, schedule const& sched,
    std::vector<intermodal::individual_modes_container::taxi>& taxis) {
  if (req.request_type()->request_options_type() !=
      RequestOptions_LateConnectionReq) {
    throw std::system_error(error::failure);
  }
  auto ops = reinterpret_cast<LateConnectionReq const*>(
      req.request_type()->request_options());
  auto const destination_eva = req.request()->destination()->id()->str();
  auto const it = sched.eva_to_station_.find(destination_eva);
  if (it == sched.eva_to_station_.end()) {
    throw std::system_error(motis::access::error::station_not_found);
  }

  ask_lookup_module(destination_eva, it->second->lat(), it->second->lng(),
                    ops->taxi_radius(), taxis);
}
}  // namespace detail

module::msg_ptr search(ReliableRoutingRequest const& req,
                       std::string const& hotels_file, schedule const& sched) {
  using namespace motis::reliability::intermodal;
  individual_modes_container container;
  parse_hotels(hotels_file, container.hotels_);
  detail::init_taxi(req, sched, container.taxis_);

  flatbuffers::request_builder b(req);
  b.add_additional_edges(container);
  return motis_call(b.build_routing_request())->val();
}
}  // namespace late_connections
}  // namespace search
}  // namespace reliability
}  // namespace motis
