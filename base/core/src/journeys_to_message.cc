#include "motis/core/journey/journeys_to_message.h"

#include "boost/date_time/gregorian/gregorian_types.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"

#include "motis/protocol/RoutingResponse_generated.h"

using namespace flatbuffers;
using namespace motis::module;
using namespace motis::routing;

namespace motis {

std::vector<Offset<Stop>> convert_stops(
    FlatBufferBuilder& b, std::vector<journey::stop> const& stops) {
  std::vector<Offset<Stop>> buf_stops;

  for (auto const& stop : stops) {
    auto arr = CreateEventInfo(
        b, stop.arrival.valid ? stop.arrival.timestamp : 0,
        stop.arrival.valid ? stop.arrival.schedule_timestamp : 0,
        b.CreateString(stop.arrival.platform));
    auto dep = CreateEventInfo(
        b, stop.departure.valid ? stop.departure.timestamp : 0,
        stop.departure.valid ? stop.departure.schedule_timestamp : 0,
        b.CreateString(stop.departure.platform));
    buf_stops.push_back(CreateStop(b, b.CreateString(stop.eva_no),
                                   b.CreateString(stop.name), stop.lat,
                                   stop.lng, arr, dep, static_cast<uint8_t>(stop.interchange)));
  }

  return buf_stops;
}

std::vector<Offset<MoveWrapper>> convert_moves(
    FlatBufferBuilder& b, std::vector<journey::transport> const& transports) {
  std::vector<Offset<MoveWrapper>> moves;

  for (auto const& t : transports) {
    Range r(t.from, t.to);
    switch (t.type) {
      case journey::transport::Walk: {
        moves.push_back(
            CreateMoveWrapper(b, Move_Walk, CreateWalk(b, &r).Union()));
        break;
      }
      case journey::transport::PublicTransport: {
        moves.push_back(CreateMoveWrapper(
            b, Move_Transport,
            CreateTransport(b, &r, b.CreateString(t.category_name),
                            t.category_id, t.clasz, t.train_nr,
                            b.CreateString(t.line_identifier),
                            b.CreateString(t.name), b.CreateString(t.provider),
                            b.CreateString(t.direction), t.route_id)
                .Union()));
        break;
      }
      case journey::transport::Mumo: {
        moves.push_back(CreateMoveWrapper(
            b, Move_Mumo,
            CreateMumo(b, &r, b.CreateString(t.mumo_type_name), t.mumo_price)
                .Union()));
        break;
      }
    }
  }

  return moves;
}

std::vector<Offset<Attribute>> convert_attributes(
    FlatBufferBuilder& b, std::vector<journey::attribute> const& attributes) {
  std::vector<Offset<Attribute>> buf_attributes;
  for (auto const& a : attributes) {
    buf_attributes.push_back(CreateAttribute(
        b, a.from, a.to, b.CreateString(a.code), b.CreateString(a.text)));
  }
  return buf_attributes;
}

Offset<routing::Connection> to_connection(flatbuffers::FlatBufferBuilder& b,
                                          journey const& j) {
  return CreateConnection(b, b.CreateVector(convert_stops(b, j.stops)),
                          b.CreateVector(convert_moves(b, j.transports)),
                          b.CreateVector(convert_attributes(b, j.attributes)),
                          0);
}

msg_ptr journeys_to_message(std::vector<journey> const& journeys,
                            int pareto_dijkstra_time) {
  MessageCreator b;

  std::vector<Offset<Connection>> connections;
  for (auto const& j : journeys) {
    connections.push_back(to_connection(b, j));
  }

  b.CreateAndFinish(MsgContent_RoutingResponse,
                    CreateRoutingResponse(b, pareto_dijkstra_time,
                                          b.CreateVector(connections))
                        .Union());

  return make_msg(b);
}

}  // namespace motis
