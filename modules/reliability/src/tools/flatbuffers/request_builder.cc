#include "motis/reliability/tools/flatbuffers/request_builder.h"

#include "motis/core/common/constants.h"
#include "motis/core/common/date_time_util.h"
#include "motis/core/common/logging.h"
#include "motis/core/journey/journey.h"
#include "motis/core/journey/journeys_to_message.h"

#include "motis/module/error.h"

#include "motis/protocol/RoutingRequest_generated.h"

#include "motis/reliability/intermodal/individual_modes_container.h"
#include "motis/reliability/intermodal/reliable_bikesharing.h"

using namespace flatbuffers;
using namespace motis::module;

namespace motis {
namespace reliability {

request_builder::request_builder(routing::SearchType search_type)
    : search_type_(search_type),
      dep_is_intermodal_(false),
      arr_is_intermodal_(false) {
  b_.ForceDefaults(true); /* necessary to write indices 0 */
  dep_.lat_ = 0.0;
  dep_.lng_ = 0.0;
  arr_.lat_ = 0.0;
  arr_.lng_ = 0.0;
}

/* for bcg-base journeys and late connections */
request_builder::request_builder(routing::RoutingRequest const& request)
    : request_builder(request.search_type()) {
  init_start_from_routing_request(request);
  add_destination(request.destination()->name()->c_str(),
                  request.destination()->id()->c_str());
}

/* to build routing-requests (rating a journey) */
request_builder::request_builder(ReliableRoutingRequest const& request)
    : request_builder(request.request()->search_type()) {
  if (!request.dep_is_intermodal()) {
    init_start_from_routing_request(*request.request());
  } else {
    if (request.request()->start_type() != routing::Start_PretripStart) {
      throw std::system_error(module::error::malformed_msg);
    }
    auto const start = reinterpret_cast<routing::PretripStart const*>(
        request.request()->start());
    add_intermodal_start(request.dep_coord()->lat(), request.dep_coord()->lng(),
                         start->interval()->begin(), start->interval()->end());
  }

  if (!request.arr_is_intermodal()) {
    add_destination(request.request()->destination()->name()->c_str(),
                    request.request()->destination()->id()->c_str());
  } else {
    add_intermodal_destination(request.arr_coord()->lat(),
                               request.arr_coord()->lng());
  }
}

/* not intermodal */
void request_builder::init_start_from_routing_request(
    routing::RoutingRequest const& request) {
  if (request.start_type() == routing::Start_OntripStationStart) {
    auto const start =
        reinterpret_cast<routing::OntripStationStart const*>(request.start());
    add_ontrip_station_start(start->station()->name()->c_str(),
                             start->station()->id()->c_str(),
                             start->departure_time());
  } else if (request.start_type() == routing::Start_PretripStart) {
    auto const start =
        reinterpret_cast<routing::PretripStart const*>(request.start());
    add_pretrip_start(start->station()->name()->c_str(),
                      start->station()->id()->c_str(),
                      start->interval()->begin(), start->interval()->end());
  } else {
    throw std::system_error(module::error::malformed_msg);
  }
}

void request_builder::create_pretrip_start(std::string const station_name,
                                           std::string const station_id,
                                           std::time_t const interval_begin,
                                           std::time_t const interval_end) {
  motis::Interval interval(interval_begin, interval_end);
  start_.first = routing::Start_PretripStart;
  start_.second =
      routing::CreatePretripStart(
          b_, routing::CreateInputStation(b_, b_.CreateString(station_id),
                                          b_.CreateString(station_name)),
          &interval)
          .Union();
}

request_builder& request_builder::add_pretrip_start(
    std::string const& name, std::string const& id,
    std::time_t const interval_begin, std::time_t const interval_end) {
  dep_is_intermodal_ = false;
  create_pretrip_start(name, id, interval_begin, interval_end);
  return *this;
}
request_builder& request_builder::add_ontrip_station_start(
    std::string const& name, std::string const& id,
    std::time_t const ontrip_time) {
  dep_is_intermodal_ = false;
  start_.first = routing::Start_OntripStationStart;
  start_.second = routing::CreateOntripStationStart(
                      b_, routing::CreateInputStation(b_, b_.CreateString(id),
                                                      b_.CreateString(name)),
                      ontrip_time)
                      .Union();
  return *this;
}

request_builder& request_builder::add_destination(std::string const& name,
                                                  std::string const& id) {
  arr_is_intermodal_ = false;
  destination_station_ = routing::CreateInputStation(b_, b_.CreateString(id),
                                                     b_.CreateString(name));
  return *this;
}

request_builder& request_builder::add_intermodal_start(
    double const& lat, double const& lng, std::time_t const interval_begin,
    std::time_t const interval_end) {
  dep_is_intermodal_ = true;
  dep_.lat_ = lat;
  dep_.lng_ = lng;
  create_pretrip_start(STATION_START, STATION_START, interval_begin,
                       interval_end);
  return *this;
}

request_builder& request_builder::add_intermodal_destination(
    double const& lat, double const& lng) {
  arr_is_intermodal_ = true;
  arr_.lat_ = lat;
  arr_.lng_ = lng;
  destination_station_ = routing::CreateInputStation(
      b_, b_.CreateString(STATION_END), b_.CreateString(STATION_END));
  return *this;
}

request_builder& request_builder::add_additional_edges(
    intermodal::individual_modes_container const& container) {
  create_bikesharing_edges(container);
  create_taxi_edges(container);
  create_hotel_edges(container);
  create_walks(container);

  LOG(logging::info) << "Request created with "
                     << container.bikesharing_at_start_.size() << "|"
                     << container.bikesharing_at_destination_.size()
                     << " bikesharings, " << container.hotels_.size()
                     << " hotels, " << container.taxis_.size() << " taxis, and "
                     << container.walks_at_start_.size() << "|"
                     << container.walks_at_destination_.size() << " walks.";
  return *this;
}

Offset<routing::RoutingRequest> request_builder::create_routing_request() {
  std::vector<::flatbuffers::Offset<routing::Via>> vias;
  return routing::CreateRoutingRequest(
      b_, start_.first, start_.second, destination_station_, search_type_,
      b_.CreateVector(vias), b_.CreateVector(additional_edges_));
}

msg_ptr request_builder::build_routing_request() {
  b_.create_and_finish(MsgContent_RoutingRequest,
                       create_routing_request().Union(), "/routing");
  return module::make_msg(b_);
}

msg_ptr request_builder::build_reliable_search_request(
    int16_t const min_dep_diff, bool const reliable_bikesharing,
    bool const unreliable_bikesharing, bool const walks) {
  auto opts = CreateRequestOptionsWrapper(
      b_, RequestOptions_ReliableSearchReq,
      CreateReliableSearchReq(b_, min_dep_diff).Union());
  return build_reliable_request(opts, reliable_bikesharing,
                                unreliable_bikesharing, walks);
}

msg_ptr request_builder::build_rating_request(bool const reliable_bikesharing,
                                              bool const unreliable_bikesharing,
                                              bool const walks) {
  auto opts = CreateRequestOptionsWrapper(b_, RequestOptions_RatingReq,
                                          CreateRatingReq(b_).Union());
  return build_reliable_request(opts, reliable_bikesharing,
                                unreliable_bikesharing, walks);
}

msg_ptr request_builder::build_late_connection_request(
    journey const& orig_journey, unsigned const taxi_radius,
    uint16_t const hotel_earliest_checkout, uint16_t const hotel_min_stay,
    uint16_t const hotel_price) {
  auto opts = CreateRequestOptionsWrapper(
      b_, RequestOptions_LateConnectionReq,
      CreateLateConnectionReq(b_, to_connection(b_, orig_journey), taxi_radius,
                              hotel_earliest_checkout, hotel_min_stay,
                              hotel_price)
          .Union());
  return build_reliable_request(opts, false, false, false);
}

msg_ptr request_builder::build_connection_tree_request(
    int16_t const num_alternatives_at_stop, int16_t const min_dep_diff) {
  auto opts = CreateRequestOptionsWrapper(
      b_, RequestOptions_ConnectionTreeReq,
      CreateConnectionTreeReq(b_, num_alternatives_at_stop, min_dep_diff)
          .Union());
  return build_reliable_request(opts, false, false, false);
}

msg_ptr request_builder::build_reliable_request(
    Offset<RequestOptionsWrapper> const& options,
    bool const reliable_bikesharing, bool const unreliable_bikesharing,
    bool const walks) {
  IndividualModes modes(reliable_bikesharing, unreliable_bikesharing, walks);
  Coordinates dep(dep_.lat_, dep_.lng_), arr(arr_.lat_, arr_.lng_);
  b_.create_and_finish(MsgContent_ReliableRoutingRequest,
                       CreateReliableRoutingRequest(
                           b_, create_routing_request(), dep_is_intermodal_,
                           arr_is_intermodal_, &dep, &arr, options, &modes)
                           .Union(),
                       "/reliability/route");
  return module::make_msg(b_);
}

void request_builder::create_bikesharing_edges(
    intermodal::individual_modes_container const& container) {
  auto create_edge = [&](
      motis::reliability::intermodal::bikesharing::bikesharing_info const& info,
      std::string const tail_station, std::string const head_station,
      int const id) {
    using namespace routing;
    for (auto const& availability : info.availability_intervals_) {
      Interval interval(availability.from_, availability.to_);
      additional_edges_.push_back(CreateAdditionalEdgeWrapper(
          b_, AdditionalEdge_TimeDependentMumoEdge,
          CreateTimeDependentMumoEdge(
              b_, CreateMumoEdge(b_, b_.CreateString(tail_station),
                                 b_.CreateString(head_station), info.duration(),
                                 0 /* TODO(Mohammad Keyhani) price */, id),
              &interval)
              .Union()));
    }
  };

  for (auto const& info : container.bikesharing_at_start_) {
    create_edge(info.second, STATION_START, info.second.station_eva_,
                info.first);
  }
  for (auto const& info : container.bikesharing_at_destination_) {
    create_edge(info.second, info.second.station_eva_, STATION_END, info.first);
  }
}

void request_builder::create_taxi_edges(
    intermodal::individual_modes_container const& container) {
  using namespace routing;
  for (auto const& e : container.taxis_) {
    Interval interval(e.second.valid_from_, e.second.valid_to_);
    additional_edges_.push_back(CreateAdditionalEdgeWrapper(
        b_, AdditionalEdge_PeriodicMumoEdge,
        CreatePeriodicMumoEdge(
            b_, CreateMumoEdge(b_, b_.CreateString(e.second.from_station_),
                               b_.CreateString(e.second.to_station_),
                               e.second.duration_, e.second.price_, e.first),
            &interval)
            .Union()));
  }
}

void request_builder::create_hotel_edges(
    intermodal::individual_modes_container const& container) {
  using namespace routing;
  for (auto const& e : container.hotels_) {
    additional_edges_.push_back(CreateAdditionalEdgeWrapper(
        b_, AdditionalEdge_HotelEdge,
        CreateHotelEdge(
            b_, CreateMumoEdge(b_, b_.CreateString(e.second.station_),
                               b_.CreateString(e.second.station_),
                               0 /* dummy */, e.second.price_, e.first),
            e.second.earliest_checkout_, e.second.min_stay_duration_)
            .Union()));
  }
}

void request_builder::create_walks(
    intermodal::individual_modes_container const& container) {
  auto create_edge = [&](
      intermodal::individual_modes_container::walk const& walk,
      std::string const tail_station, std::string const head_station,
      int const id) {
    using namespace routing;
    additional_edges_.push_back(CreateAdditionalEdgeWrapper(
        b_, AdditionalEdge_MumoEdge,
        CreateMumoEdge(b_, b_.CreateString(tail_station),
                       b_.CreateString(head_station), walk.duration_,
                       0 /* price */, id)
            .Union()));

  };
  for (auto const& walk : container.walks_at_start_) {
    create_edge(walk.second, STATION_START, walk.second.station_id_,
                walk.first);
  }
  for (auto const& walk : container.walks_at_destination_) {
    create_edge(walk.second, walk.second.station_id_, STATION_END, walk.first);
  }
}

std::string departure_station_name(routing::RoutingRequest const& req) {
  using namespace routing;
  if (req.start_type() == Start_PretripStart) {
    auto start = reinterpret_cast<PretripStart const*>(req.start());
    return start->station()->name()->str();
  } else if (req.start_type() == Start_OntripStationStart) {
    auto start = reinterpret_cast<OntripStationStart const*>(req.start());
    return start->station()->name()->str();
  }
  return "";
}

}  // namespace reliability
}  // namespace motis
