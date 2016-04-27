#include "motis/reliability/tools/flatbuffers/request_builder.h"

#include "motis/core/common/date_time_util.h"

#include "motis/module/error.h"

#include "motis/reliability/intermodal/reliable_bikesharing.h"

using namespace flatbuffers;
using namespace motis::module;

namespace motis {
namespace reliability {
namespace flatbuffers {

request_builder::request_builder(routing::Type const type,
                                 routing::Direction const dir)
    : type_(type),
      direction_(dir),
      interval_begin_(0),
      interval_end_(0),
      is_intermodal_(false) {
  b_.ForceDefaults(true); /* necessary to write indices 0 */
  dep_.lat_ = 0.0;
  dep_.lng_ = 0.0;
  arr_.lat_ = 0.0;
  arr_.lng_ = 0.0;
}

request_builder::request_builder(routing::RoutingRequest const* request)
    : request_builder(request->type(), request->direction()) {
  set_interval(request->interval()->begin(), request->interval()->end());
  for (auto const& e : *request->path()) {
    add_station(e->name() ? e->name()->c_str() : "",
                e->eva_nr() ? e->eva_nr()->c_str() : "");
  }
}

request_builder& request_builder::add_station(std::string const& name,
                                              std::string const& eva) {
  if (is_intermodal_) {
    throw std::system_error(error::malformed_msg);
  }
  path_.push_back(routing::CreateStationPathElement(b_, b_.CreateString(name),
                                                    b_.CreateString(eva)));
  return *this;
}

request_builder& request_builder::add_dep_coordinates(double const& lat,
                                                      double const& lng) {
  is_intermodal_ = true;
  dep_.lat_ = lat;
  dep_.lng_ = lng;
  return *this;
}

// for intermodal requests
request_builder& request_builder::add_arr_coordinates(double const& lat,
                                                      double const& lng) {
  is_intermodal_ = true;
  arr_.lat_ = lat;
  arr_.lng_ = lng;
  return *this;
}

request_builder& request_builder::set_interval(std::time_t const begin,
                                               std::time_t const end) {
  interval_begin_ = begin;
  interval_end_ = end;
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
  // TODO(Mohammad Keyhani) Peridic edges for taxi

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
                                 0 /* TODO(Mohammad Keyhani) price */,
                                 0 /* TODO(Mohammad Keyhani) slot */),
              interval.first, interval.second)
              .Union()));
    }
  };

  for (auto const& info : infos.at_start_) {
    create_edge(info, "START", info.station_eva_);
  }

  for (auto const& info : infos.at_destination_) {
    create_edge(info, info.station_eva_, "END");
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
  Coordinates dep(dep_.lat_, dep_.lng_), arr(arr_.lat_, arr_.lng_);
  b_.create_and_finish(
      MsgContent_ReliableRoutingRequest,
      CreateReliableRoutingRequest(b_, create_routing_request(), is_intermodal_,
                                   &dep, &arr, options, &modes)
          .Union(),
      "/reliability/route");
  return module::make_msg(b_);
}

}  // namespace flatbuffers
}  // namespace reliability
}  // namespace motis
