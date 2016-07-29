#include "motis/intermodal/intermodal.h"

#include "motis/core/common/constants.h"
#include "motis/module/context/motis_call.h"
#include "motis/loader/util.h"

#include "motis/intermodal/error.h"

#include "motis/protocol/Message_generated.h"

using namespace flatbuffers;
using namespace motis::module;
using namespace motis::lookup;
using namespace motis::osrm;
using namespace motis::loader;
using namespace motis::routing;

namespace motis {
namespace intermodal {

intermodal::intermodal() : module("Intermodal Options", "intermodal") {}

intermodal::~intermodal() = default;

void intermodal::init(motis::module::registry& r) {
  r.register_op("/intermodal", [this](msg_ptr const& m) { return route(m); });
}

msg_ptr make_geo_request(Position const* pos, double radius) {
  message_creator mc;
  mc.create_and_finish(
      MsgContent_LookupGeoStationRequest,
      CreateLookupGeoStationRequest(mc, pos, 0, radius).Union(),
      "/lookup/geo_station");
  return make_msg(mc);
}

msg_ptr make_osrm_request(Position const* pos,
                          Vector<Offset<Station>> const* stations,
                          char const* profile, Direction direction) {
  std::vector<Position> many;
  for (auto const* station : *stations) {
    many.push_back(*station->pos());
  }

  message_creator mc;
  mc.create_and_finish(
      MsgContent_OSRMOneToManyRequest,
      CreateOSRMOneToManyRequest(mc, mc.CreateString(profile), direction, pos,
                                 mc.CreateVectorOfStructs(many))
          .Union(),
      "/osrm/one_to_many");
  return make_msg(mc);
}

void add_start_edges(message_creator& mc, IntermodalRoutingRequest const* req,
                     Position const* start_pos,
                     std::vector<Offset<AdditionalEdgeWrapper>>& edges) {
  for (auto const& wrapper : *req->start_modes()) {
    switch (wrapper->mode_type()) {
      case Mode_Walk: {
        auto max_dur =
            reinterpret_cast<Walk const*>(wrapper->mode())->max_duration();
        auto max_dist = max_dur * WALK_SPEED;

        auto geo_msg = motis_call(make_geo_request(start_pos, max_dist))->val();
        auto geo_resp = motis_content(LookupGeoStationResponse, geo_msg);

        auto osrm_msg =
            motis_call(make_osrm_request(start_pos, geo_resp->stations(),
                                         "foot", Direction_Forward))
                ->val();
        auto osrm_resp = motis_content(OSRMOneToManyResponse, osrm_msg);

        for (auto i = 0ul; i < geo_resp->stations()->size(); ++i) {
          auto const walk_dur = osrm_resp->costs()->Get(i)->time();
          if (walk_dur > max_dur) {
            continue;
          }

          edges.push_back(CreateAdditionalEdgeWrapper(
              mc, AdditionalEdge_MumoEdge,
              CreateMumoEdge(
                  mc, mc.CreateString(STATION_START),
                  mc.CreateString(geo_resp->stations()->Get(i)->id()->str()),
                  walk_dur / 60, 0, 0)
                  .Union()));
        }
      } break;
      default: throw std::system_error(error::unknown_mode);
    }
  }
}

void add_destination_edges(message_creator& mc,
                           IntermodalRoutingRequest const* req,
                           std::vector<Offset<AdditionalEdgeWrapper>>& edges) {
  auto const* destination = req->destination();
  for (auto const& wrapper : *req->destination_modes()) {
    switch (wrapper->mode_type()) {
      case Mode_Walk: {
        auto max_dur =
            reinterpret_cast<Walk const*>(wrapper->mode())->max_duration();
        auto max_dist = max_dur * WALK_SPEED;

        auto geo_msg =
            motis_call(make_geo_request(destination, max_dist))->val();
        auto geo_resp = motis_content(LookupGeoStationResponse, geo_msg);

        auto osrm_msg =
            motis_call(make_osrm_request(destination, geo_resp->stations(),
                                         "foot", Direction_Backward))
                ->val();
        auto osrm_resp = motis_content(OSRMOneToManyResponse, osrm_msg);

        for (auto i = 0ul; i < geo_resp->stations()->size(); ++i) {
          auto const walk_dur = osrm_resp->costs()->Get(i)->time();
          if (walk_dur > max_dur) {
            continue;
          }

          edges.push_back(CreateAdditionalEdgeWrapper(
              mc, AdditionalEdge_MumoEdge,
              CreateMumoEdge(mc, mc.CreateString(
                                     geo_resp->stations()->Get(i)->id()->str()),
                             mc.CreateString(STATION_END), walk_dur / 60, 0, 0)
                  .Union()));
        }
      } break;
      default: throw std::system_error(error::unknown_mode);
    }
  }
}

msg_ptr intermodal::route(msg_ptr const& msg) {
  auto const req = motis_content(IntermodalRoutingRequest, msg);
  message_creator mc;

  Start start_type;
  Offset<void> routing_start;
  Position const* start_pos;

  auto start_station = CreateInputStation(mc, mc.CreateString(STATION_START),
                                          mc.CreateString(STATION_START));
  switch (req->start_type()) {
    case IntermodalStart_IntermodalOntripStart: {
      auto start = reinterpret_cast<IntermodalOntripStart const*>(req->start());
      start_type = Start_OntripStationStart;
      routing_start =
          CreateOntripStationStart(mc, start_station, start->departure_time())
              .Union();
      start_pos = start->position();
    } break;
    case IntermodalStart_IntermodalPretripStart: {
      auto start =
          reinterpret_cast<IntermodalPretripStart const*>(req->start());
      start_type = Start_PretripStart;
      routing_start =
          CreatePretripStart(mc, start_station, start->interval()).Union();
      start_pos = start->position();
    } break;
    default: throw std::system_error(error::unknown_start);
  }

  std::vector<Offset<AdditionalEdgeWrapper>> edges;
  add_start_edges(mc, req, start_pos, edges);
  add_destination_edges(mc, req, edges);

  mc.create_and_finish(
      MsgContent_RoutingRequest,
      CreateRoutingRequest(mc, start_type, routing_start,
                           CreateInputStation(mc, mc.CreateString(STATION_END),
                                              mc.CreateString(STATION_END)),
                           req->search_type(), req->search_dir(),
                           mc.CreateVector(std::vector<Offset<Via>>{}),
                           mc.CreateVector(edges))
          .Union(),
      "/routing");
  return motis_call(make_msg(mc))->val();
}

}  // namespace intermodal
}  // namespace motis
