#include "motis/routes/routes.h"

#include "boost/filesystem.hpp"

#include "parser/file.h"

#include "motis/core/access/trip_access.h"
#include "motis/core/access/trip_iterator.h"
#include "motis/core/access/trip_section.h"
#include "motis/module/context/get_schedule.h"
#include "motis/module/context/motis_call.h"

#include "motis/routes/fbs/RoutesAuxiliary_generated.h"

namespace fs = boost::filesystem;
using namespace flatbuffers;
using namespace parser;
using namespace motis::module;
using namespace motis::osrm;
using namespace motis::access;

namespace motis {
namespace routes {

routes::routes() : module("Routes", "routes") {
  string_param(aux_file_, "routes-auxiliary.raw", "aux",
               "/path/to/routes-auxiliary.raw");
};
routes::~routes() = default;

void routes::init(registry& r) {
  if (fs::is_regular_file(aux_file_)) {
    load_auxiliary_file();
  }

  r.register_op("/routes/id_train",
                [this](msg_ptr const& m) { return id_train_routes(m); });
}

void routes::load_auxiliary_file() {
  auto const buf = file(aux_file_.c_str(), "r").content();
  auto const aux_content = GetRoutesAuxiliary(buf.buf_);

  for (auto const& s : *aux_content->bus_stop_positions()) {
    extra_bus_stop_positions_[s->station_id()->str()].emplace_back(s->lat(),
                                                                   s->lng());
  }
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

msg_ptr routes::trip_to_osrm_request(schedule const& sched, trip const* trp) {
  message_creator mc;

  std::vector<Offset<Waypoint>> waypoints;
  for (auto const& stop : access::stops(trp)) {
    auto const& station = stop.get_station(sched);

    std::vector<Position> pos;
    pos.emplace_back(station.lat(), station.lng());

    auto it = extra_bus_stop_positions_.find(station.eva_nr_);
    if (it != end(extra_bus_stop_positions_)) {
      for (auto const& pair : it->second) {
        pos.emplace_back(pair.first, pair.second);
      }
    }
    waypoints.emplace_back(CreateWaypoint(mc, mc.CreateVectorOfStructs(pos)));
  }

  mc.create_and_finish(
      MsgContent_OSRMSmoothViaRouteRequest,
      CreateOSRMSmoothViaRouteRequest(mc, mc.CreateString("car"),
                                      mc.CreateVector(waypoints))
          .Union(),
      "/osrm/smooth_via");
  return make_msg(mc);
}

msg_ptr routes::resolve_route_osrm(schedule const& sched, trip const* trp) {
  auto osrm_req = trip_to_osrm_request(sched, trp);
  auto const osrm_msg = motis_call(osrm_req)->val();
  auto osrm_resp = motis_content(OSRMSmoothViaRouteResponse, osrm_msg);

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
