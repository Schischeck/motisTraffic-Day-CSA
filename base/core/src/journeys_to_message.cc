#include "motis/core/journey/journeys_to_message.h"

#include "boost/date_time/gregorian/gregorian_types.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"

#include "motis/protocol/RoutingResponse_generated.h"

using namespace flatbuffers;
using namespace motis::module;
using namespace motis::routing;

namespace motis {

TimestampReason convert_reason(delay_info::reason const r) {
  switch (r) {
    case delay_info::reason::SCHEDULE: return TimestampReason_SCHEDULE;
    case delay_info::reason::IS: return TimestampReason_IS;
    case delay_info::reason::FORECAST: return TimestampReason_FORECAST;
    case delay_info::reason::PROPAGATION: return TimestampReason_PROPAGATION;
    default: return TimestampReason_SCHEDULE;
  }
}

std::vector<Offset<Stop>> convert_stops(
    FlatBufferBuilder& b, std::vector<journey::stop> const& stops) {
  std::vector<Offset<Stop>> buf_stops;

  for (auto const& stop : stops) {
    auto const arr = CreateEventInfo(
        b, stop.arrival_.valid_ ? stop.arrival_.timestamp_ : 0,
        stop.arrival_.valid_ ? stop.arrival_.schedule_timestamp_ : 0,
        b.CreateString(stop.arrival_.platform_),
        convert_reason(stop.arrival_.timestamp_reason_));
    auto const dep = CreateEventInfo(
        b, stop.departure_.valid_ ? stop.departure_.timestamp_ : 0,
        stop.departure_.valid_ ? stop.departure_.schedule_timestamp_ : 0,
        b.CreateString(stop.departure_.platform_),
        convert_reason(stop.departure_.timestamp_reason_));
    auto const pos = Position(stop.lat_, stop.lng_);
    buf_stops.push_back(
        CreateStop(b, CreateStation(b, b.CreateString(stop.eva_no_),
                                    b.CreateString(stop.name_), &pos),
                   arr, dep, static_cast<uint8_t>(stop.interchange_)));
  }

  return buf_stops;
}

std::vector<Offset<MoveWrapper>> convert_moves(
    FlatBufferBuilder& b, std::vector<journey::transport> const& transports) {
  std::vector<Offset<MoveWrapper>> moves;

  for (auto const& t : transports) {
    Range r(t.from_, t.to_);
    if (t.is_walk_) {
      moves.push_back(CreateMoveWrapper(
          b, Move_Walk, CreateWalk(b, &r, t.slot_, t.mumo_price_,
                                   b.CreateString(t.mumo_type_))
                            .Union()));
    } else {
      moves.push_back(CreateMoveWrapper(
          b, Move_Transport,
          CreateTransport(b, &r, b.CreateString(t.category_name_),
                          t.category_id_, t.clasz_, t.train_nr_,
                          b.CreateString(t.line_identifier_),
                          b.CreateString(t.name_), b.CreateString(t.provider_),
                          b.CreateString(t.direction_))
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
        b, a.from_, a.to_, b.CreateString(a.code_), b.CreateString(a.text_)));
  }
  return buf_attributes;
}

Offset<Connection> to_connection(FlatBufferBuilder& b, journey const& j) {
  return CreateConnection(b, b.CreateVector(convert_stops(b, j.stops_)),
                          b.CreateVector(convert_moves(b, j.transports_)),
                          b.CreateVector(convert_attributes(b, j.attributes_)),
                          j.night_penalty_, j.db_costs_);
}

msg_ptr journeys_to_message(std::vector<journey> const& journeys,
                            int pareto_dijkstra_time) {
  message_creator b;

  std::vector<Offset<Connection>> connections;
  for (auto const& j : journeys) {
    connections.push_back(to_connection(b, j));
  }

  b.create_and_finish(MsgContent_RoutingResponse,
                      CreateRoutingResponse(b, pareto_dijkstra_time,
                                            b.CreateVector(connections))
                          .Union());

  return make_msg(b);
}

}  // namespace motis
