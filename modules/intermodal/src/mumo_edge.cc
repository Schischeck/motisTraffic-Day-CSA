#include "motis/intermodal/mumo_edge.h"

#include <algorithm>

#include "motis/core/common/constants.h"
#include "motis/core/common/erase_if.h"
#include "motis/module/context/motis_call.h"
#include "motis/module/message.h"

#include "motis/intermodal/error.h"

using namespace geo;
using namespace flatbuffers;
using namespace motis::routing;
using namespace motis::lookup;
using namespace motis::osrm;
using namespace motis::module;

namespace motis {
namespace intermodal {

msg_ptr make_geo_request(latlng const& pos, double radius) {
  Position fbs_position{pos.lat_, pos.lng_};
  message_creator mc;
  mc.create_and_finish(
      MsgContent_LookupGeoStationRequest,
      CreateLookupGeoStationRequest(mc, &fbs_position, 0, radius).Union(),
      "/lookup/geo_station");
  return make_msg(mc);
}

msg_ptr make_osrm_request(latlng const& pos,
                          Vector<Offset<Station>> const* stations,
                          std::string const& profile, Direction direction) {
  Position fbs_position{pos.lat_, pos.lng_};
  std::vector<Position> many;
  for (auto const* station : *stations) {
    many.push_back(*station->pos());
  }

  message_creator mc;
  mc.create_and_finish(
      MsgContent_OSRMOneToManyRequest,
      CreateOSRMOneToManyRequest(mc, mc.CreateString(profile), direction,
                                 &fbs_position, mc.CreateVectorOfStructs(many))
          .Union(),
      "/osrm/one_to_many");
  return make_msg(mc);
}

void osrm_edges(latlng const& pos, int max_dur, int max_dist,
                mumo_type const type, Direction direction,
                appender_fun const& appender) {
  auto geo_msg = motis_call(make_geo_request(pos, max_dist))->val();
  auto geo_resp = motis_content(LookupGeoStationResponse, geo_msg);
  auto stations = geo_resp->stations();

  auto osrm_msg =
      motis_call(make_osrm_request(pos, stations, to_string(type), direction))
          ->val();
  auto osrm_resp = motis_content(OSRMOneToManyResponse, osrm_msg);

  for (auto i = 0ul; i < stations->size(); ++i) {
    auto const dur = osrm_resp->costs()->Get(i)->duration();
    if (dur > max_dur) {
      continue;
    }

    appender(stations->Get(i)->id()->str(), dur / 60, type);
  }
}

void make_edges(Vector<Offset<ModeWrapper>> const* modes, latlng const& pos,
                Direction const osrm_direction, appender_fun const& appender) {
  for (auto const& wrapper : *modes) {
    switch (wrapper->mode_type()) {
      case Mode_Walk: {
        auto max_dur =
            reinterpret_cast<Walk const*>(wrapper->mode())->max_duration();
        auto max_dist = max_dur * WALK_SPEED;
        osrm_edges(pos, max_dur, max_dist, mumo_type::FOOT, osrm_direction,
                   appender);

      } break;
      case Mode_Bike: {
        auto max_dur =
            reinterpret_cast<Bike const*>(wrapper->mode())->max_duration();
        auto max_dist = max_dur * BIKE_SPEED;
        osrm_edges(pos, max_dur, max_dist, mumo_type::BIKE, osrm_direction,
                   appender);

      } break;
      default: throw std::system_error(error::unknown_mode);
    }
  }
}

void make_starts(IntermodalRoutingRequest const* req, latlng const& pos,
                 appender_fun const& appender) {
  make_edges(req->start_modes(), pos, Direction_Forward, appender);
}

void make_dests(IntermodalRoutingRequest const* req, latlng const& pos,
                appender_fun const& appender) {
  make_edges(req->destination_modes(), pos, Direction_Backward, appender);
}

void remove_intersection(std::vector<mumo_edge>& starts,
                         std::vector<mumo_edge> const& destinations,
                         SearchDir const dir) {
  if (dir == SearchDir_Forward) {
    for (auto const& dest : destinations) {
      erase_if(starts,
               [&dest](auto const& start) { return start.to_ == dest.from_; });
    }
  } else {
    for (auto const& dest : destinations) {
      erase_if(starts,
               [&dest](auto const& start) { return start.from_ == dest.to_; });
    }
  }
}

std::vector<Offset<AdditionalEdgeWrapper>> write_edges(
    FlatBufferBuilder& fbb, std::vector<mumo_edge> const& starts,
    std::vector<mumo_edge> const& destinations) {
  std::vector<Offset<AdditionalEdgeWrapper>> edges;

  for (auto const& edge : starts) {
    edges.push_back(CreateAdditionalEdgeWrapper(
        fbb, AdditionalEdge_MumoEdge,
        CreateMumoEdge(fbb, fbb.CreateString(edge.from_),
                       fbb.CreateString(edge.to_), edge.duration_, 0,
                       to_int(edge.type_))
            .Union()));
  }

  for (auto const& edge : destinations) {
    edges.push_back(CreateAdditionalEdgeWrapper(
        fbb, AdditionalEdge_MumoEdge,
        CreateMumoEdge(fbb, fbb.CreateString(edge.from_),
                       fbb.CreateString(edge.to_), edge.duration_, 0,
                       to_int(edge.type_))
            .Union()));
  }

  return edges;
}

}  // namespace motis
}  // namespace motis
