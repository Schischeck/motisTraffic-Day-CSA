#include "motis/routes/routes.h"

#include "boost/filesystem.hpp"

#include "motis/core/access/trip_access.h"
#include "motis/core/access/trip_iterator.h"
#include "motis/core/access/trip_section.h"
#include "motis/module/context/get_schedule.h"
#include "motis/module/context/motis_call.h"


using namespace flatbuffers;
using namespace motis::module;
using namespace motis::osrm;
using namespace motis::access;

namespace motis {
namespace routes {

routes::routes() : module("Routes", "routes"){};
routes::~routes() = default;

void routes::init(registry& r) {
  r.register_op("/routes/id_train",
                [this](msg_ptr const& m) { return id_train_routes(m); });
}

msg_ptr routes::id_train_routes(msg_ptr const& msg) {
  auto const& req = motis_content(RoutesIdTrainRequest, msg);
  auto const& sched = get_schedule();
  auto const& trp = get_trip(sched, req->trip_id());

  switch (trip_section{trp, 0}.fcon().clasz_) {
    case MOTIS_STR:
    case MOTIS_BUS:  //
      return resolve_route_osrm(sched, trp);
    default:  //
      return resolve_route_stub(sched, trp);
  }
}

msg_ptr trip_to_osrm_request(schedule const& sched, trip const* trp) {
  message_creator mc;
  std::vector<Position> waypoints;
  for (auto const& stop : access::stops(trp)) {
    auto const& station = stop.get_station(sched);
    waypoints.push_back({station.lat(), station.lng()});
  }
  mc.create_and_finish(
      MsgContent_OSRMViaRouteRequest,
      CreateOSRMViaRouteRequest(mc, mc.CreateString("car"),
                                mc.CreateVectorOfStructs(waypoints))
          .Union(),
      "/osrm/via");
  return make_msg(mc);
}

msg_ptr routes::resolve_route_osrm(schedule const& sched, trip const* trp) {
  auto osrm_req = trip_to_osrm_request(sched, trp);
  auto const osrm_msg = motis_call(osrm_req)->val();
  auto osrm_resp = motis_content(OSRMViaRouteResponse, osrm_msg);

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
                                  mc.CreateString("osrm/car"))
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
