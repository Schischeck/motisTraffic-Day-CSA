#include "motis/reliability/tools/flatbuffers/request_builder.h"

#include "motis/core/common/date_util.h"

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
    add_station(e->name() ? e->name()->c_str() : "",
                e->eva_nr() ? e->eva_nr()->c_str() : "");
  }
}

request_builder& request_builder::add_station(std::string const& name,
                                              std::string const& eva) {
  stations_.push_back(routing::CreateStationPathElement(
      b_, b_.CreateString(name), b_.CreateString(eva)));
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

Offset<routing::RoutingRequest> request_builder::create_routing_request() {
  routing::Interval interval(interval_begin_, interval_end_);
  return routing::CreateRoutingRequest(b_, &interval, type_, direction_,
                                       b_.CreateVector(stations_),
                                       b_.CreateVector(additional_edges_));
}

msg_ptr request_builder::build_routing_request() {
  b_.CreateAndFinish(MsgContent_RoutingRequest,
                     create_routing_request().Union());
  return module::make_msg(b_);
}

msg_ptr request_builder::build_reliable_search_request(short const min_dep_diff) {
  auto opts = reliability::CreateRequestOptionsWrapper(
      b_, reliability::RequestOptions_ReliableSearchReq,
      reliability::CreateReliableSearchReq(b_, min_dep_diff).Union());
  return build_reliable_request(opts);
}

msg_ptr request_builder::build_rating_request() {
  auto opts = reliability::CreateRequestOptionsWrapper(
      b_, reliability::RequestOptions_RatingReq,
      reliability::CreateRatingReq(b_).Union());
  return build_reliable_request(opts);
}

msg_ptr request_builder::build_late_connection_cequest() {
  auto opts = reliability::CreateRequestOptionsWrapper(
      b_, reliability::RequestOptions_LateConnectionReq,
      reliability::CreateLateConnectionReq(b_).Union());
  return build_reliable_request(opts);
}

msg_ptr request_builder::build_connection_tree_request(
    short const num_alternatives_at_stop, short const min_dep_diff) {
  auto opts = reliability::CreateRequestOptionsWrapper(
      b_, reliability::RequestOptions_ConnectionTreeReq,
      reliability::CreateConnectionTreeReq(b_, num_alternatives_at_stop,
                                           min_dep_diff)
          .Union());
  return build_reliable_request(opts);
}

msg_ptr request_builder::build_reliable_request(
    Offset<RequestOptionsWrapper> const& options) {
  IndividualModes modes(false, false);
  b_.CreateAndFinish(MsgContent_ReliableRoutingRequest,
                     reliability::CreateReliableRoutingRequest(
                         b_, create_routing_request(), options, &modes)
                         .Union());
  return module::make_msg(b_);
}

}  // namespace request_builder
}  // namespace flatbuffers
}  // namespace reliability
}  // namespace motis
