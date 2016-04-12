#include "motis/reliability/intermodal/reliable_bikesharing.h"

#include <memory>
#include <vector>

#include "motis/module/module.h"

#include "motis/reliability/reliability.h"

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
        availability_intervals.emplace_back((time_t)rating->begin(),
                                            (time_t)rating->end());
      }
    }
    if (!availability_intervals.empty()) {
      infos.push_back(
          {std::string(edge->eva_nr()->c_str()),
           (unsigned int)((edge->bike_duration() + edge->walk_duration()) / 60),
           availability_intervals, std::string(edge->from()->name()->c_str()),
           std::string(edge->to()->name()->c_str())});
    }
  }
  return infos;
}

void handle_bikesharing_response(
    motis::module::msg_ptr msg, boost::system::error_code e,
    std::shared_ptr<availability_aggregator const> aggregator, callback cb) {
  if (msg == nullptr || e != nullptr) {
    throw boost::system::system_error(e);
  }
  auto const* response =
      msg->content<::motis::bikesharing::BikesharingResponse const*>();
  return cb(bikesharing_infos{
      to_bikesharing_infos(response->departure_edges(), aggregator),
      to_bikesharing_infos(response->arrival_edges(), aggregator)});
}
}  // namespace detail

void retrieve_bikesharing_infos(
    module::msg_ptr request,
    std::shared_ptr<availability_aggregator> aggregator,
    reliability& reliability_module, module::sid session_id, callback cb) {
  reliability_module.send_message(
      request, session_id,
      std::bind(&detail::handle_bikesharing_response, std::placeholders::_1,
                std::placeholders::_2, aggregator, cb));
}

module::msg_ptr to_bikesharing_request(
    double const departure_lat, double const departure_lng,
    double const arrival_lat, double const arrival_lng, time_t window_begin,
    time_t window_end, motis::bikesharing::AvailabilityAggregator aggregator) {
  module::MessageCreator fb;
  fb.CreateAndFinish(MsgContent_BikesharingRequest,
                     motis::bikesharing::CreateBikesharingRequest(
                         fb, departure_lat, departure_lng, arrival_lat,
                         arrival_lng, window_begin, window_end, aggregator)
                         .Union());
  return module::make_msg(fb);
}

module::msg_ptr to_bikesharing_request(
    routing::RoutingRequest const* req,
    motis::bikesharing::AvailabilityAggregator aggregator) {
  if (req->path()->size() != 2 ||
      req->path()->Get(0)->element_type() !=
          routing::LocationPathElement_CoordinatesPathElement ||
      req->path()->Get(1)->element_type() !=
          routing::LocationPathElement_CoordinatesPathElement) {
    throw boost::system::system_error(boost::system::error_code());
  }
  auto start = reinterpret_cast<routing::CoordinatesPathElement const*>(
      req->path()->Get(0)->element());
  auto destination = reinterpret_cast<routing::CoordinatesPathElement const*>(
      req->path()->Get(1)->element());

  return to_bikesharing_request(start->lat(), start->lon(), destination->lat(),
                                destination->lon(), req->interval()->begin(),
                                req->interval()->end(), aggregator);
}

}  // namespace bikesharing
}  // namespace intermodal
}  // namespace reliability
}  // namespace motis
