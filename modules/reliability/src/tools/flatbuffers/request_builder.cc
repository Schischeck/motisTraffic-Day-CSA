#include "motis/reliability/tools/flatbuffers/request_builder.h"

#include "motis/core/common/date_time_util.h"

#include "motis/reliability/intermodal/reliable_bikesharing.h"

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
}

request_builder::request_builder(routing::Type const type,
                                 routing::Direction const dir)
    : type_(type), direction_(dir), interval_begin_(0), interval_end_(0) {
  b_.ForceDefaults(true); /* necessary to write indices 0 */
}

request_builder::request_builder(routing::RoutingRequest const* request)
    : request_builder(request->type(), request->direction()) {
  set_interval(request->interval()->begin(), request->interval()->end());
  for (auto const& e : *request->path()) {
    if (e->element_type() == routing::LocationPathElement_StationPathElement) {
      auto st =
          reinterpret_cast<routing::StationPathElement const*>(e->element());
      add_station(st->name() ? st->name()->c_str() : "",
                  st->eva_nr() ? st->eva_nr()->c_str() : "");
    } else if (e->element_type() ==
               routing::LocationPathElement_CoordinatesPathElement) {
      auto c = reinterpret_cast<routing::CoordinatesPathElement const*>(
          e->element());
      add_coordinates(c->lat(), c->lon(), c->is_source());
    }
  }
}

request_builder& request_builder::add_station(std::string const& name,
                                              std::string const& eva) {
  path_.push_back(routing::CreateLocationPathElementWrapper(
      b_, routing::LocationPathElement_StationPathElement,
      routing::CreateStationPathElement(b_, b_.CreateString(name),
                                        b_.CreateString(eva))
          .Union()));
  return *this;
}

request_builder& request_builder::add_coordinates(double const& lat,
                                                  double const& lon,
                                                  bool const is_source) {
  path_.push_back(routing::CreateLocationPathElementWrapper(
      b_, routing::LocationPathElement_CoordinatesPathElement,
      routing::CreateCoordinatesPathElement(b_, lat, lon, is_source).Union()));
  return *this;
}

request_builder& request_builder::set_interval(std::time_t const begin,
                                               std::time_t const end) {
  interval_begin_ = begin;
  interval_end_ = end;
  return *this;
}

request_builder& request_builder::set_interval(
    std::tuple<int, int, int> const ddmmyyyy, motis::time const begin,
    motis::time const end) {
  interval_begin_ = detail::to_unix_time(ddmmyyyy, begin);
  interval_end_ = detail::to_unix_time(ddmmyyyy, end);
  return *this;
}

request_builder& request_builder::add_additional_edge(
    Offset<routing::AdditionalEdgeWrapper> const& edge) {
  additional_edges_.push_back(edge);
  return *this;
}

request_builder& request_builder::add_additional_edges(
    motis::reliability::intermodal::bikesharing::bikesharing_infos const&
        infos) {
  auto create_edge = [&](
      motis::reliability::intermodal::bikesharing::bikesharing_info const& info,
      std::string const tail_station, std::string const head_station) {
    using namespace routing;
    for (auto const& interval : info.availability_intervals_) {
      additional_edges_.push_back(CreateAdditionalEdgeWrapper(
          b_, AdditionalEdge_TimeDependentMumoEdge,
          CreateTimeDependentMumoEdge(
              b_, CreateMumoEdge(b_, b_.CreateString(tail_station),
                                 b_.CreateString(head_station), info.duration_,
                                 0 /* TODO(Mohammad Keyhani) slots */),
              interval.first, interval.second, 0)
              .Union()));
    }
  };

  for (auto const& info : infos.at_start_) {
    create_edge(info, "-1", info.station_eva_);
  }

  for (auto const& info : infos.at_destination_) {
    create_edge(info, info.station_eva_, "-2");
  }
  return *this;
}

Offset<routing::RoutingRequest> request_builder::create_routing_request() {
  routing::Interval interval(interval_begin_, interval_end_);
  return routing::CreateRoutingRequest(b_, &interval, type_, direction_,
                                       b_.CreateVector(path_),
                                       b_.CreateVector(additional_edges_));
}

msg_ptr request_builder::build_routing_request() {
  b_.create_and_finish(MsgContent_RoutingRequest,
                       create_routing_request().Union(), "/routing");
  return module::make_msg(b_);
}

msg_ptr request_builder::build_reliable_search_request(short const min_dep_diff,
                                                       bool const bikesharing) {
  auto opts = CreateRequestOptionsWrapper(
      b_, RequestOptions_ReliableSearchReq,
      CreateReliableSearchReq(b_, min_dep_diff).Union());
  return build_reliable_request(opts, bikesharing);
}

msg_ptr request_builder::build_rating_request(bool const bikesharing) {
  auto opts = CreateRequestOptionsWrapper(b_, RequestOptions_RatingReq,
                                          CreateRatingReq(b_).Union());
  return build_reliable_request(opts, bikesharing);
}

msg_ptr request_builder::build_late_connection_cequest() {
  auto opts = CreateRequestOptionsWrapper(b_, RequestOptions_LateConnectionReq,
                                          CreateLateConnectionReq(b_).Union());
  return build_reliable_request(opts);
}

msg_ptr request_builder::build_connection_tree_request(
    short const num_alternatives_at_stop, short const min_dep_diff) {
  auto opts = CreateRequestOptionsWrapper(
      b_, RequestOptions_ConnectionTreeReq,
      CreateConnectionTreeReq(b_, num_alternatives_at_stop, min_dep_diff)
          .Union());
  return build_reliable_request(opts);
}

msg_ptr request_builder::build_reliable_request(
    Offset<RequestOptionsWrapper> const& options, bool const bikesharing) {
  IndividualModes modes(bikesharing, 0);
  b_.create_and_finish(MsgContent_ReliableRoutingRequest,
                       CreateReliableRoutingRequest(
                           b_, create_routing_request(), options, &modes)
                           .Union(),
                       "/routing");
  return module::make_msg(b_);
}

}  // namespace request_builder
}  // namespace flatbuffers
}  // namespace reliability
}  // namespace motis
