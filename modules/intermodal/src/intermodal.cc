#include "motis/intermodal/intermodal.h"

#include <functional>

#include "motis/core/common/constants.h"
#include "motis/core/journey/journeys_to_message.h"
#include "motis/core/journey/message_to_journeys.h"
#include "motis/module/context/motis_call.h"

#include "motis/intermodal/error.h"
#include "motis/intermodal/mumo_edge.h"
#include "motis/intermodal/query_bounds.h"

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

msg_ptr postprocess_response(msg_ptr response_msg, query_start const& q_start,
                             query_dest const& q_dest, SearchDir const dir) {
  auto routing_response = motis_content(RoutingResponse, response_msg);
  auto journeys = message_to_journeys(routing_response);

  message_creator mc;
  std::vector<Offset<Connection>> processed_connections_journeys_;
  for (auto& journey : journeys) {
    auto& stops = journey.stops_;
    if (stops.size() < 2) {
      continue;
    }

    if (q_start.is_intermodal_) {
      auto& start = (dir == SearchDir_Forward) ? stops.front() : stops.back();
      start.lat_ = q_start.pos_.lat_;
      start.lng_ = q_start.pos_.lng_;
    }

    if (q_dest.is_intermodal_) {
      auto& dest = (dir == SearchDir_Forward) ? stops.back() : stops.front();
      dest.lat_ = q_dest.pos_.lat_;
      dest.lng_ = q_dest.pos_.lng_;
    }

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

  auto const start = parse_query_start(mc, req);
  auto const dest = parse_query_dest(mc, req);

  auto appender = [](auto& vec, auto const& from, auto const& to,
                     auto const dur, mumo_type const type) {
    vec.emplace_back(from, to, dur, type);
  };

  std::vector<mumo_edge> deps;
  std::vector<mumo_edge> arrs;

  using namespace std::placeholders;
  if (req->search_dir() == SearchDir_Forward) {
    if (start.is_intermodal_) {
      make_starts(req, start.pos_, std::bind(appender, std::ref(deps),  //
                                             STATION_START, _1, _2, _3));
    }
    if (dest.is_intermodal_) {
      make_dests(req, dest.pos_, std::bind(appender, std::ref(arrs),  //
                                           _1, STATION_END, _2, _3));
    }
  } else {
    if (start.is_intermodal_) {
      make_starts(req, start.pos_, std::bind(appender, std::ref(deps),  //
                                             _1, STATION_START, _2, _3));
    }
    if (dest.is_intermodal_) {
      make_dests(req, dest.pos_, std::bind(appender, std::ref(arrs),  //
                                           STATION_END, _1, _2, _3));
    }
  }

  remove_intersection(deps, arrs, req->search_dir());
  auto edges = write_edges(mc, deps, arrs);

  mc.create_and_finish(
      MsgContent_RoutingRequest,
      CreateRoutingRequest(mc, start.start_type_, start.start_, dest.station_,
                           req->search_type(), req->search_dir(),
                           mc.CreateVector(std::vector<Offset<Via>>{}),
                           mc.CreateVector(edges))
          .Union(),
      "/routing");

  auto resp = motis_call(make_msg(mc))->val();
  return postprocess_response(resp, start, dest, req->search_dir());
}

}  // namespace intermodal
}  // namespace motis
