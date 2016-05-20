#include "motis/reliability/intermodal/reliable_bikesharing.h"

#include <memory>
#include <vector>

#include "motis/module/context/motis_call.h"
#include "motis/module/module.h"

#include "motis/protocol/ReliableRoutingRequest_generated.h"
#include "motis/protocol/RoutingRequest_generated.h"

#include "motis/reliability/error.h"

namespace motis {
namespace reliability {
namespace intermodal {
namespace bikesharing {
namespace detail {

std::vector<bikesharing_info> const to_bikesharing_infos(
    ::flatbuffers::Vector<::flatbuffers::Offset<
        ::motis::bikesharing::BikesharingEdge>> const* edges,
    std::shared_ptr<availability_aggregator const> aggregator) {
  std::vector<bikesharing_info> infos;
  for (auto edge : *edges) {
    std::vector<std::pair<time_t, time_t>> availability_intervals;
    for (auto rating : *edge->availability()) {
      if (aggregator->is_reliable(rating->value())) {
        availability_intervals.emplace_back(
            static_cast<time_t>(rating->begin()),
            static_cast<time_t>(rating->end()));
      }
    }
    if (!availability_intervals.empty()) {
      infos.push_back(
          {std::string(edge->station_id()->c_str()),
           static_cast<unsigned>(
               (edge->bike_duration() + edge->walk_duration()) / 60),
           availability_intervals, std::string(edge->from()->name()->c_str()),
           std::string(edge->to()->name()->c_str())});
    }
  }
  return infos;
}

}  // namespace detail

bikesharing_infos retrieve_bikesharing_infos(
    module::msg_ptr request,
    std::shared_ptr<availability_aggregator> aggregator) {
  using ::motis::bikesharing::BikesharingResponse;

  auto res = motis_call(request)->val();

  auto const response = motis_content(BikesharingResponse, res);
  return bikesharing_infos{
      detail::to_bikesharing_infos(response->departure_edges(), aggregator),
      detail::to_bikesharing_infos(response->arrival_edges(), aggregator)};
}

module::msg_ptr to_bikesharing_request(
    ReliableRoutingRequest const* req,
    motis::bikesharing::AvailabilityAggregator aggregator) {
  if (req->request()->start_type() != routing::Start_PretripStart) {
    throw std::system_error(error::not_implemented);
  }
  auto start =
      reinterpret_cast<routing::PretripStart const*>(req->request()->start());
  return to_bikesharing_request(
      req->dep_coord()->lat(), req->dep_coord()->lng(), req->arr_coord()->lat(),
      req->arr_coord()->lng(), start->interval()->begin(),
      start->interval()->end(), aggregator);
}

module::msg_ptr to_bikesharing_request(
    double const departure_lat, double const departure_lng,
    double const arrival_lat, double const arrival_lng, time_t window_begin,
    time_t window_end, motis::bikesharing::AvailabilityAggregator aggregator) {
  module::message_creator fb;
  Position dep(departure_lat, departure_lng), arr(arrival_lat, arrival_lng);
  Interval window(window_begin, window_end);
  fb.create_and_finish(MsgContent_BikesharingRequest,
                       motis::bikesharing::CreateBikesharingRequest(
                           fb, &dep, &arr, &window, aggregator)
                           .Union(),
                       "/bikesharing");
  return module::make_msg(fb);
}

}  // namespace bikesharing
}  // namespace intermodal
}  // namespace reliability
}  // namespace motis
