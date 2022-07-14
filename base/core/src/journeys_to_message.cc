#include "motis/core/journey/journeys_to_message.h"

#include "boost/date_time/gregorian/gregorian_types.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"

#include "motis/core/conv/trip_conv.h"

using namespace flatbuffers;
using namespace motis::module;
using namespace motis::routing;

namespace motis {

TimestampReason convert_reason(timestamp_reason const r) {
  switch (r) {
    case timestamp_reason::SCHEDULE: return TimestampReason_SCHEDULE;
    case timestamp_reason::IS: return TimestampReason_IS;
    case timestamp_reason::FORECAST: return TimestampReason_FORECAST;
    case timestamp_reason::PROPAGATION: return TimestampReason_PROPAGATION;
    default: return TimestampReason_SCHEDULE;
  }
}

std::vector<Offset<Stop>> convert_stops(
    FlatBufferBuilder& b, std::vector<journey::stop> const& stops) {
  std::vector<Offset<Stop>> buf_stops;

  for (auto const& stop : stops) {
    auto const arr =
        stop.arrival_.valid_
            ? CreateEventInfo(b, stop.arrival_.timestamp_,
                              stop.arrival_.schedule_timestamp_,
                              b.CreateString(stop.arrival_.track_),
                              b.CreateString(stop.arrival_.schedule_track_),
                              stop.arrival_.valid_,
                              convert_reason(stop.arrival_.timestamp_reason_))
            : CreateEventInfo(b, 0, 0, b.CreateString(""), b.CreateString(""),
                              stop.arrival_.valid_, TimestampReason_SCHEDULE);
    auto const dep =
        stop.departure_.valid_
            ? CreateEventInfo(b, stop.departure_.timestamp_,
                              stop.departure_.schedule_timestamp_,
                              b.CreateString(stop.departure_.track_),
                              b.CreateString(stop.departure_.schedule_track_),
                              stop.departure_.valid_,
                              convert_reason(stop.departure_.timestamp_reason_))
            : CreateEventInfo(b, 0, 0, b.CreateString(""), b.CreateString(""),
                              stop.departure_.valid_, TimestampReason_SCHEDULE);
    auto const pos = Position(stop.lat_, stop.lng_);
    buf_stops.push_back(
        CreateStop(b,
                   CreateStation(b, b.CreateString(stop.eva_no_),
                                 b.CreateString(stop.name_), &pos),
                   arr, dep, static_cast<uint8_t>(stop.exit_) != 0u,
                   static_cast<uint8_t>(stop.enter_) != 0u));
  }

  return buf_stops;
}

std::vector<Offset<MoveWrapper>> convert_moves(
    FlatBufferBuilder& b, std::vector<journey::transport> const& transports) {
  std::vector<Offset<MoveWrapper>> moves;

  for (auto const& t : transports) {
    Range r(t.from_, t.to_);
    if (t.is_walk_) {
      moves.push_back(
          CreateMoveWrapper(b, Move_Walk,
                            CreateWalk(b, &r, t.mumo_id_, t.mumo_price_, 0,
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

std::vector<Offset<Trip>> convert_trips(
    FlatBufferBuilder& b, std::vector<journey::trip> const& trips) {
  std::vector<Offset<Trip>> journey_trips;

  for (auto const& t : trips) {
    auto const r =
        Range{static_cast<int16_t>(t.from_), static_cast<int16_t>(t.to_)};
    journey_trips.push_back(
        CreateTrip(b, &r,
                   CreateTripId(b, b.CreateString(t.station_id_), t.train_nr_,
                                t.time_, b.CreateString(t.target_station_id_),
                                t.target_time_, b.CreateString(t.line_id_)),
                   b.CreateString("")));
  }

  return journey_trips;
}

std::vector<Offset<Attribute>> convert_attributes(
    FlatBufferBuilder& b, std::vector<journey::attribute> const& attributes) {
  std::vector<Offset<Attribute>> buf_attributes;
  for (auto const& a : attributes) {
    auto const r =
        Range{static_cast<int16_t>(a.from_), static_cast<int16_t>(a.to_)};
    buf_attributes.push_back(CreateAttribute(b, &r, b.CreateString(a.code_),
                                             b.CreateString(a.text_)));
  }
  return buf_attributes;
}

Offset<Connection> to_connection(FlatBufferBuilder& b, journey const& j) {
  std::vector<Offset<Problem>> i_am_empty;
  std::vector<Offset<FreeText>> i_am_free;
  return CreateConnection(b, b.CreateVector(convert_stops(b, j.stops_)),
                          b.CreateVector(convert_moves(b, j.transports_)),
                          b.CreateVector(convert_trips(b, j.trips_)),
                          b.CreateVector(convert_attributes(b, j.attributes_)),
                          b.CreateVector(i_am_free), b.CreateVector(i_am_empty),
                          j.night_penalty_, j.db_costs_);
}

}  // namespace motis
