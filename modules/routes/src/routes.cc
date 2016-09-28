#include "motis/routes/routes.h"

#include <memory>

#include "motis/core/access/trip_access.h"
#include "motis/core/access/trip_iterator.h"
#include "motis/core/access/trip_section.h"
#include "motis/module/context/get_schedule.h"
#include "motis/module/context/motis_call.h"

#include "motis/routes/auxiliary_data.h"

using namespace flatbuffers;
using namespace motis::module;
using namespace motis::osrm;
using namespace motis::access;
using namespace motis::geo;

namespace motis {
namespace routes {

routes::routes()
    : module("Routes", "routes"), aux_(std::make_unique<auxiliary_data>()) {
  string_param(aux_file_, "routes-auxiliary.raw", "aux",
               "/path/to/routes-auxiliary.raw");
};
routes::~routes() = default;

void routes::init(registry& r) {
  aux_->load(aux_file_);

  r.register_op("/routes/id_train",
                [this](msg_ptr const& m) { return id_train_routes(m); });
}

msg_ptr routes::id_train_routes(msg_ptr const& msg) {
  auto const& req = motis_content(RoutesIdTrainRequest, msg);
  auto const& sched = get_schedule();
  auto const& t = req->trip_id();
  auto const& trp = get_trip(sched, t->station_id()->str(), t->train_nr(),
                             t->time(), t->target_station_id()->str(),
                             t->target_time(), t->line_id()->str());

  auto const prepared = resolve_prepared_route(sched, trp);
  if (prepared) {
    auto const& msg = *prepared;

    auto copy = std::make_shared<message>(msg->size());
    // copy->buffer_size_ = msg->size();
    // copy->buffer_ = flatbuffers::unique_ptr_t(
    //     reinterpret_cast<uint8_t*>(operator new(msg->size())),
    //     std::default_delete<uint8_t>());
    std::memcpy(copy->buffer_.get(), msg->data(), msg->size());

    return copy;
  }

  switch (trip_section{trp, 0}.fcon().clasz_) {
    case MOTIS_STR:
    case MOTIS_BUS:  //
      return resolve_route_osrm(sched, trp);
    default:  //
      return resolve_route_stub(sched, trp);
  }
}

boost::optional<msg_ptr> routes::resolve_prepared_route(schedule const& sched,
                                                        trip const* trp) {
  auto const seq = transform_to_vec(
      access::stops(trp),
      [&sched](auto const& stop) { return stop.get_station(sched).eva_nr_; });
  auto const clasz = trip_section{trp, 0}.fcon().clasz_;

  auto it = aux_->prepared_routes_.find({seq, clasz});
  return it != end(aux_->prepared_routes_) ? it->second
                                           : boost::optional<msg_ptr>{};
}

msg_ptr routes::trip_to_osrm_request(schedule const& sched, trip const* trp) {
  message_creator mc;

  std::vector<Offset<Waypoint>> waypoints;
  for (auto const& stop : access::stops(trp)) {
    auto const& station = stop.get_station(sched);

    std::vector<motis::Position> pos;
    pos.emplace_back(station.lat(), station.lng());

    auto it = aux_->extra_stop_positions_.find(station.eva_nr_);
    if (it != end(aux_->extra_stop_positions_)) {
      for (auto const& p : it->second) {
        pos.emplace_back(p.lat_, p.lng_);
      }
    }
    waypoints.emplace_back(CreateWaypoint(mc, mc.CreateVectorOfStructs(pos)));
  }

  mc.create_and_finish(
      MsgContent_OSRMSmoothViaRouteRequest,
      CreateOSRMSmoothViaRouteRequest(mc, mc.CreateString("bus"),
                                      mc.CreateVector(waypoints))
          .Union(),
      "/osrm/smooth_via");
  return make_msg(mc);
}

msg_ptr routes::resolve_route_osrm(schedule const& sched, trip const* trp) {
  auto const osrm_req = trip_to_osrm_request(sched, trp);
  auto const osrm_msg = motis_call(osrm_req)->val();
  auto const osrm_resp = motis_content(OSRMSmoothViaRouteResponse, osrm_msg);

  message_creator mc;
  std::vector<Offset<Polyline>> segments;
  for (auto const& s : *osrm_resp->segments()) {
    segments.push_back(CreatePolyline(
        mc,
        mc.CreateVector(s->coordinates()->data(), s->coordinates()->size())));
  }
  mc.create_and_finish(
      MsgContent_RoutesIdTrainResponse,
      CreateRoutesIdTrainResponse(mc, mc.CreateVector(segments),
                                  mc.CreateString("osrm/bus"))
          .Union());
  return make_msg(mc);
}

msg_ptr routes::resolve_route_stub(schedule const& sched, trip const* trp) {
  message_creator mc;
  std::vector<Offset<Polyline>> segments;
  for (auto const& section : access::sections(trp)) {
    std::vector<double> segment;
    auto const& from = section.from_station(sched);
    segment.push_back(from.lat());
    segment.push_back(from.lng());

    auto const& to = section.to_station(sched);
    segment.push_back(to.lat());
    segment.push_back(to.lng());

    segments.push_back(CreatePolyline(mc, mc.CreateVector(segment)));
  }
  mc.create_and_finish(
      MsgContent_RoutesIdTrainResponse,
      CreateRoutesIdTrainResponse(mc, mc.CreateVector(segments),
                                  mc.CreateString("stub"))
          .Union());
  return make_msg(mc);
}

}  // namespace routes
}  // namespace motis
