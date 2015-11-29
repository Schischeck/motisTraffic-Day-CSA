#include "motis/routing/response_builder.h"

#include "boost/date_time/gregorian/gregorian_types.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"

#include "motis/protocol/RoutingResponse_generated.h"

using namespace flatbuffers;
using namespace motis::module;

namespace motis {
namespace routing {

namespace detail {
std::vector<Offset<Stop>> convert_stops(
    FlatBufferBuilder& b, std::vector<journey::stop> const& stops) {
  std::vector<Offset<Stop>> buf_stops;

  for (auto const& stop : stops) {
    auto arr =
        CreateEventInfo(b, stop.arrival.valid ? stop.arrival.timestamp : 0,
                        b.CreateString(stop.arrival.platform));
    auto dep =
        CreateEventInfo(b, stop.departure.valid ? stop.departure.timestamp : 0,
                        b.CreateString(stop.departure.platform));
    buf_stops.push_back(CreateStop(b, b.CreateString(stop.eva_no),
                                   b.CreateString(stop.name), arr, dep,
                                   stop.interchange));
  }

  return buf_stops;
}

std::vector<Offset<MoveWrapper>> convert_moves(
    FlatBufferBuilder& b, std::vector<journey::transport> const& transports) {
  std::vector<Offset<MoveWrapper>> moves;

  for (auto const& t : transports) {
    Range r(t.from, t.to);
    if (t.walk) {
      moves.push_back(
          CreateMoveWrapper(b, Move_Walk, CreateWalk(b, &r).Union()));
    } else {
      moves.push_back(CreateMoveWrapper(
          b, Move_Transport,
          CreateTransport(b, &r, b.CreateString(t.category_name), t.category_id,
                          t.clasz, t.train_nr,
                          b.CreateString(t.line_identifier),
                          b.CreateString(t.name), b.CreateString(t.provider),
                          b.CreateString(t.direction), t.route_id)
              .Union()));
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
}  // namespace detail

motis::module::msg_ptr journeys_to_message(
    std::vector<journey> const& journeys) {
  MessageCreator b;

  std::vector<Offset<Connection>> connections;
  for (auto const& j : journeys) {
    connections.push_back(CreateConnection(
        b, b.CreateVector(detail::convert_stops(b, j.stops)),
        b.CreateVector(detail::convert_moves(b, j.transports)),
        b.CreateVector(detail::convert_attributes(b, j.attributes))));
  }

  b.CreateAndFinish(
      MsgContent_RoutingResponse,
      CreateRoutingResponse(b, b.CreateVector(connections)).Union());

  return make_msg(b);
}

}  // namespace routing
}  // namespace motis
