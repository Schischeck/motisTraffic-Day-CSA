#include "motis/core/journey/message_to_journeys.h"

#include <algorithm>

#include "motis/core/schedule/category.h"
#include "motis/core/conv/timestamp_reason_conv.h"
#include "motis/core/journey/journey.h"
#include "motis/core/journey/journey_util.h"

#include "motis/protocol/RoutingResponse_generated.h"

namespace motis {

journey::stop::event_info to_event_info(EventInfo const& event,
                                        bool const valid) {
  journey::stop::event_info e;
  e.track_ = event.track()->c_str();
  e.timestamp_ = event.time();
  e.schedule_timestamp_ = event.schedule_time();
  e.timestamp_reason_ = from_fbs(event.reason());
  e.valid_ = valid;
  return e;
}

journey::stop to_stop(Stop const& stop, unsigned int const index,
                      unsigned int const num_stops) {
  journey::stop s;
  s.eva_no_ = stop.station()->id()->c_str();
  s.exit_ = static_cast<bool>(stop.exit());
  s.enter_ = static_cast<bool>(stop.enter());
  s.lat_ = stop.station()->pos()->lat();
  s.lng_ = stop.station()->pos()->lng();
  s.name_ = stop.station()->name()->c_str();
  s.arrival_ = to_event_info(*stop.arrival(), index != 0);
  s.departure_ = to_event_info(*stop.departure(), index + 1 != num_stops);
  return s;
}

journey::transport create_empty_transport() {
  journey::transport t;
  t.category_id_ = 0;
  t.from_ = 0;
  t.mumo_price_ = 0;
  t.mumo_id_ = 0;
  t.to_ = 0;
  t.train_nr_ = 0;
  t.is_walk_ = false;
  return t;
}

journey::trip create_empty_trip() {
  journey::trip t;
  t.from_ = 0;
  t.to_ = 0;
  t.train_nr_ = 0;
  t.time_ = 0;
  t.target_time_ = 0;
  return t;
}

journey::transport to_transport(Walk const& walk, time duration) {
  auto t = create_empty_transport();
  t.is_walk_ = true;
  t.duration_ = duration;
  t.from_ = walk.range()->from();
  t.to_ = walk.range()->to();
  t.mumo_id_ = walk.mumo_id();
  t.mumo_price_ = walk.price();
  t.mumo_type_ = walk.mumo_type()->c_str();
  return t;
}

journey::transport to_transport(Transport const& transport, time duration) {
  auto t = create_empty_transport();
  t.duration_ = duration;
  t.from_ = transport.range()->from();
  t.to_ = transport.range()->to();
  t.is_walk_ = false;
  t.category_name_ = transport.category_name()->c_str();
  t.category_id_ = transport.category_id();
  t.clasz_ = transport.clasz();
  t.direction_ = transport.direction()->c_str();
  t.line_identifier_ = transport.line_id()->c_str();
  t.name_ = transport.name()->c_str();
  t.provider_ = transport.provider()->c_str();
  t.mumo_id_ = 0;
  t.train_nr_ = transport.train_nr();
  return t;
}

journey::trip to_trip(Trip const& trip) {
  auto t = create_empty_trip();
  t.from_ = trip.range()->from();
  t.to_ = trip.range()->to();
  t.station_id_ = trip.id()->station_id()->str();
  t.train_nr_ = trip.id()->train_nr();
  t.time_ = trip.id()->time();
  t.target_station_id_ = trip.id()->target_station_id()->str();
  t.target_time_ = trip.id()->target_time();
  t.line_id_ = trip.id()->line_id()->str();
  return t;
}

journey::attribute to_attribute(Attribute const& attribute) {
  journey::attribute a;
  a.code_ = attribute.code()->c_str();
  a.text_ = attribute.text()->c_str();
  a.from_ = attribute.range()->from();
  a.to_ = attribute.range()->to();
  return a;
}

time get_move_duration(
    Range const& range,
    flatbuffers::Vector<flatbuffers::Offset<Stop>> const& stops) {
  Stop const& from = *stops[range.from()];
  Stop const& to = *stops[range.to()];
  return time((to.arrival()->time() - from.departure()->time()) / 60);
}

journey convert(Connection const* conn) {
  journey journey;

  /* stops */
  unsigned int stop_index = 0;
  for (auto const& stop : *conn->stops()) {
    journey.stops_.push_back(
        to_stop(*stop, stop_index++, conn->stops()->size()));
  }

  /* transports */
  for (auto const& move : *conn->transports()) {
    if (move->move_type() == Move_Walk) {
      auto const walk = reinterpret_cast<Walk const*>(move->move());
      journey.transports_.push_back(to_transport(
          *walk, get_move_duration(*walk->range(), *conn->stops())));
    } else if (move->move_type() == Move_Transport) {
      auto const transport = reinterpret_cast<Transport const*>(move->move());
      journey.transports_.push_back(to_transport(
          *transport, get_move_duration(*transport->range(), *conn->stops())));
    }
  }

  /* attributes */
  for (auto const& attribute : *conn->attributes()) {
    journey.attributes_.push_back(to_attribute(*attribute));
  }

  /* trips */
  for (auto const& trp : *conn->trips()) {
    journey.trips_.push_back(to_trip(*trp));
  }

  journey.duration_ = get_duration(journey);
  journey.transfers_ = get_transfers(journey);
  journey.night_penalty_ = conn->night_penalty();
  journey.db_costs_ = conn->db_costs();

  return journey;
}

std::vector<journey> message_to_journeys(
    routing::RoutingResponse const* response) {
  std::vector<journey> journeys;
  for (auto conn : *response->connections()) {
    journeys.push_back(convert(conn));
  }
  return journeys;
}

}  // namespace motis
