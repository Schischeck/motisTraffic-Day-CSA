#include "motis/intermodal/intermodal.h"

#include <functional>

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

template <typename Fn>
void add_departure(message_creator& mc, IntermodalRoutingRequest const* req,
                   Position const* start_pos, Fn appender) {
  for (auto const& wrapper : *req->start_modes()) {
    switch (wrapper->mode_type()) {
      case Mode_Walk: {
        auto max_dur =
            reinterpret_cast<Walk const*>(wrapper->mode())->max_duration();
        auto max_dist = max_dur * WALK_SPEED;

        auto geo_msg = motis_call(make_geo_request(start_pos, max_dist))->val();
        auto geo_resp = motis_content(LookupGeoStationResponse, geo_msg);
        auto stations = geo_resp->stations();

        auto osrm_msg = motis_call(make_osrm_request(start_pos, stations,
                                                     "foot", Direction_Forward))
                            ->val();
        auto osrm_resp = motis_content(OSRMOneToManyResponse, osrm_msg);

        for (auto i = 0ul; i < stations->size(); ++i) {
          auto const walk_dur = osrm_resp->costs()->Get(i)->time();
          if (walk_dur > max_dur) {
            continue;
          }

          appender(mc.CreateString(stations->Get(i)->id()->str()),
                   walk_dur / 60);
        }
      } break;
      default: throw std::system_error(error::unknown_mode);
    }
  }
}

template <typename Fn>
void add_arrival(message_creator& mc, IntermodalRoutingRequest const* req,
                 Position const* end_pos, Fn appender) {
  for (auto const& wrapper : *req->destination_modes()) {
    switch (wrapper->mode_type()) {
      case Mode_Walk: {
        auto max_dur =
            reinterpret_cast<Walk const*>(wrapper->mode())->max_duration();
        auto max_dist = max_dur * WALK_SPEED;

        auto geo_msg = motis_call(make_geo_request(end_pos, max_dist))->val();
        auto geo_resp = motis_content(LookupGeoStationResponse, geo_msg);
        auto stations = geo_resp->stations();

        auto osrm_msg = motis_call(make_osrm_request(end_pos, stations, "foot",
                                                     Direction_Backward))
                            ->val();
        auto osrm_resp = motis_content(OSRMOneToManyResponse, osrm_msg);

        for (auto i = 0ul; i < stations->size(); ++i) {
          auto const walk_dur = osrm_resp->costs()->Get(i)->time();
          if (walk_dur > max_dur) {
            continue;
          }

          appender(mc.CreateString(stations->Get(i)->id()->str()),
                   walk_dur / 60);
        }
      } break;
      default: throw std::system_error(error::unknown_mode);
    }
  }
}

struct query_start {
  query_start(message_creator& mc, IntermodalRoutingRequest const* req) {
    auto start_station = CreateInputStation(mc, mc.CreateString(STATION_START),
                                            mc.CreateString(STATION_START));
    switch (req->start_type()) {
      case IntermodalStart_IntermodalOntripStart: {
        auto start =
            reinterpret_cast<IntermodalOntripStart const*>(req->start());
        type_ = Start_OntripStationStart;
        transformed_ =
            CreateOntripStationStart(mc, start_station, start->departure_time())
                .Union();
        pos_ = start->position();
      } break;
      case IntermodalStart_IntermodalPretripStart: {
        auto start =
            reinterpret_cast<IntermodalPretripStart const*>(req->start());
        type_ = Start_PretripStart;
        transformed_ =
            CreatePretripStart(mc, start_station, start->interval()).Union();
        pos_ = start->position();
      } break;
      default: throw std::system_error(error::unknown_start);
    }
  }

  Start type_;
  Offset<void> transformed_;
  Position const* pos_;
};

msg_ptr intermodal::route(msg_ptr const& msg) {
  auto const req = motis_content(IntermodalRoutingRequest, msg);
  message_creator mc;

  auto const start = query_start{mc, req};
  auto const* end_pos = req->destination();

  std::vector<Offset<AdditionalEdgeWrapper>> edges;
  auto appender = [&](auto&& from, auto&& to, unsigned dur) {
    edges.push_back(CreateAdditionalEdgeWrapper(
        mc, AdditionalEdge_MumoEdge,
        CreateMumoEdge(mc, from, to, dur, 0, 0).Union()));
  };

  auto const start_node = mc.CreateString(STATION_START);
  auto const end_node = mc.CreateString(STATION_END);

  using namespace std::placeholders;
  if (req->search_dir() == SearchDir_Forward) {
    add_departure(mc, req, start.pos_, std::bind(appender, start_node, _1, _2));
    add_arrival(mc, req, end_pos, std::bind(appender, _1, end_node, _2));
  } else {
    add_departure(mc, req, end_pos, std::bind(appender, _1, start_node, _2));
    add_arrival(mc, req, start.pos_, std::bind(appender, end_node, _1, _2));
  }

  mc.create_and_finish(
      MsgContent_RoutingRequest,
      CreateRoutingRequest(mc, start.type_, start.transformed_,
                           CreateInputStation(mc, mc.CreateString(STATION_END),
                                              mc.CreateString(STATION_END)),
                           req->search_type(), SearchDir_Forward,
                           mc.CreateVector(std::vector<Offset<Via>>{}),
                           mc.CreateVector(edges))
          .Union(),
      "/routing");
  return motis_call(make_msg(mc))->val();
}

}  // namespace intermodal
}  // namespace motis
