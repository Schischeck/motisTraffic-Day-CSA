#include "motis/intermodal/intermodal.h"

#include <functional>

#include "motis/core/common/constants.h"
#include "motis/core/journey/journeys_to_message.h"
#include "motis/core/journey/message_to_journeys.h"
#include "motis/module/context/motis_call.h"

#include "motis/intermodal/error.h"
#include "motis/intermodal/mumo_edge.h"

#include "motis/protocol/Message_generated.h"

using namespace flatbuffers;
using namespace motis::module;
using namespace motis::lookup;
using namespace motis::osrm;
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
                          std::string const& profile, Direction direction) {
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
void osrm_edges(Position const* pos, int max_dur, int max_dist,
                mumo_type const type, Direction direction, Fn appender) {
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

template <typename Fn>
void make_deps(IntermodalRoutingRequest const* req, Position const* pos,
               Fn appender) {
  constexpr auto kDir = Direction_Forward;

  for (auto const& wrapper : *req->start_modes()) {
    switch (wrapper->mode_type()) {
      case Mode_Walk: {
        auto max_dur =
            reinterpret_cast<Walk const*>(wrapper->mode())->max_duration();
        auto max_dist = max_dur * WALK_SPEED;
        osrm_edges(pos, max_dur, max_dist, mumo_type::FOOT, kDir, appender);

      } break;
      case Mode_Bike: {
        auto max_dur =
            reinterpret_cast<Bike const*>(wrapper->mode())->max_duration();
        auto max_dist = max_dur * BIKE_SPEED;
        osrm_edges(pos, max_dur, max_dist, mumo_type::BIKE, kDir, appender);

      } break;
      default: throw std::system_error(error::unknown_mode);
    }
  }
}

template <typename Fn>
void make_arrs(IntermodalRoutingRequest const* req, Position const* pos,
               Fn appender) {
  constexpr auto kDir = Direction_Backward;

  for (auto const& wrapper : *req->destination_modes()) {
    switch (wrapper->mode_type()) {
      case Mode_Walk: {
        auto max_dur =
            reinterpret_cast<Walk const*>(wrapper->mode())->max_duration();
        auto max_dist = max_dur * WALK_SPEED;
        osrm_edges(pos, max_dur, max_dist, mumo_type::FOOT, kDir, appender);

      } break;
      case Mode_Bike: {
        auto max_dur =
            reinterpret_cast<Walk const*>(wrapper->mode())->max_duration();
        auto max_dist = max_dur * BIKE_SPEED;
        osrm_edges(pos, max_dur, max_dist, mumo_type::BIKE, kDir, appender);

      } break;
      default: throw std::system_error(error::unknown_mode);
    }
  }
}

struct query_start {
  Start type_;
  Offset<void> transformed_;
  Position const* pos_;
};

query_start get_query_start(message_creator& mc,
                            IntermodalRoutingRequest const* req) {
  query_start qs;
  auto start_station = CreateInputStation(mc, mc.CreateString(STATION_START),
                                          mc.CreateString(STATION_START));
  switch (req->start_type()) {
    case IntermodalStart_IntermodalOntripStart: {
      auto start = reinterpret_cast<IntermodalOntripStart const*>(req->start());
      qs.type_ = Start_OntripStationStart;
      qs.transformed_ =
          CreateOntripStationStart(mc, start_station, start->departure_time())
              .Union();
      qs.pos_ = start->position();
    } break;
    case IntermodalStart_IntermodalPretripStart: {
      auto start =
          reinterpret_cast<IntermodalPretripStart const*>(req->start());
      qs.type_ = Start_PretripStart;
      qs.transformed_ =
          CreatePretripStart(mc, start_station, start->interval()).Union();
      qs.pos_ = start->position();
    } break;
    default: throw std::system_error(error::unknown_start);
  }
  return qs;
}

msg_ptr postprocess_response(msg_ptr response_msg, Position const* start_pos,
                             Position const* dest_pos, SearchDir const dir) {
  auto routing_response = motis_content(RoutingResponse, response_msg);
  auto journeys = message_to_journeys(routing_response);

  message_creator mc;
  std::vector<Offset<Connection>> processed_connections_journeys_;
  for (auto& journey : journeys) {
    auto& stops = journey.stops_;
    if (stops.size() < 2) {
      continue;
    }

    auto& start = (dir == SearchDir_Forward) ? stops.front() : stops.back();
    start.lat_ = start_pos->lat();
    start.lng_ = start_pos->lng();

    auto& dest = (dir == SearchDir_Forward) ? stops.back() : stops.front();
    dest.lat_ = dest_pos->lat();
    dest.lng_ = dest_pos->lng();

    for (auto& t : journey.transports_) {
      if (!t.is_walk_ || t.mumo_id_ < 0) {
        continue;
      }

      t.mumo_type_ = to_string(static_cast<mumo_type>(t.mumo_id_));
      t.mumo_id_ = 0;
    }
  }

  mc.create_and_finish(
      MsgContent_IntermodalRoutingResponse,
      CreateIntermodalRoutingResponse(
          mc,
          mc.CreateVector(transform_to_vec(
              journeys, [&mc](auto const& j) { return to_connection(mc, j); })))
          .Union());

  return make_msg(mc);
}

msg_ptr intermodal::route(msg_ptr const& msg) {
  auto const req = motis_content(IntermodalRoutingRequest, msg);
  message_creator mc;

  auto const start = get_query_start(mc, req);
  auto const* dest_pos = req->destination();

  auto appender = [](auto& vec, auto const& from, auto const& to,
                     auto const dur, mumo_type const type) {
    vec.emplace_back(from, to, dur, type);
  };

  std::vector<mumo_edge> deps;
  std::vector<mumo_edge> arrs;

  using namespace std::placeholders;
  if (req->search_dir() == SearchDir_Forward) {
    make_deps(req, start.pos_,
              std::bind(appender, std::ref(deps), STATION_START, _1, _2, _3));
    make_arrs(req, dest_pos,
              std::bind(appender, std::ref(arrs), _1, STATION_END, _2, _3));
  } else {
    make_deps(req, start.pos_,
              std::bind(appender, std::ref(deps), _1, STATION_START, _2, _3));
    make_arrs(req, dest_pos,
              std::bind(appender, std::ref(arrs), STATION_END, _1, _2, _3));
  }

  remove_intersection(deps, arrs, req->search_dir());
  auto edges = write_edges(mc, deps, arrs);

  mc.create_and_finish(
      MsgContent_RoutingRequest,
      CreateRoutingRequest(mc, start.type_, start.transformed_,
                           CreateInputStation(mc, mc.CreateString(STATION_END),
                                              mc.CreateString(STATION_END)),
                           req->search_type(), req->search_dir(),
                           mc.CreateVector(std::vector<Offset<Via>>{}),
                           mc.CreateVector(edges))
          .Union(),
      "/routing");

  auto resp = motis_call(make_msg(mc))->val();
  return postprocess_response(resp, start.pos_, dest_pos, req->search_dir());
}

}  // namespace intermodal
}  // namespace motis
