#include "motis/core/journey/message_to_journeys.h"

#include <algorithm>

#include "motis/core/schedule/category.h"
#include "motis/core/journey/journey.h"
#include "motis/core/journey/journey_util.h"

#include "motis/protocol/RoutingResponse_generated.h"

namespace motis {

journey::stop::event_info to_event_info(EventInfo const& event,
                                        bool const valid) {
  journey::stop::event_info e;
  e.platform_ = event.platform()->c_str();
  e.timestamp_ = event.time();
  e.schedule_timestamp_ = event.schedule_time();
  e.valid_ = valid;
  return e;
}
journey::stop to_stop(Stop const& stop, unsigned int const index,
                      unsigned int const num_stops) {
  journey::stop s;
  s.eva_no_ = stop.station()->id()->c_str();
  s.interchange_ = static_cast<bool>(stop.interchange());
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
  t.category_name_ = "";
  t.direction_ = "";
  t.from_ = 0;
  t.line_identifier_ = "";
  t.mumo_price_ = 0;
  t.mumo_type_ = "";
  t.name_ = "";
  t.provider_ = "";
  t.slot_ = 0;
  t.to_ = 0;
  t.train_nr_ = 0;
  t.is_walk_ = false;
  return t;
}
journey::transport to_transport(Walk const& walk, uint16_t duration) {
  auto t = create_empty_transport();
  t.is_walk_ = true;
  t.duration_ = duration;
  t.from_ = walk.range()->from();
  t.to_ = walk.range()->to();
  t.slot_ = walk.slot();

  t.mumo_price_ = walk.price();
  t.mumo_type_ = walk.mumo_type()->c_str();
  return t;
}
journey::transport to_transport(Transport const& transport, uint16_t duration) {
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
  t.slot_ = 0;
  t.train_nr_ = transport.train_nr();
  return t;
}

journey::attribute to_attribute(Attribute const& attribute) {
  journey::attribute a;
  a.code_ = attribute.code()->c_str();
  a.from_ = attribute.from();
  a.text_ = attribute.text()->c_str();
  a.to_ = attribute.to();
  return a;
}

uint16_t get_move_duration(
    Range const& range,
    flatbuffers::Vector<flatbuffers::Offset<Stop>> const& stops) {
  Stop const& from = *stops[range.from()];
  Stop const& to = *stops[range.to()];
  return (to.arrival()->time() - from.departure()->time()) / 60;
}

std::vector<journey> message_to_journeys(
    routing::RoutingResponse const* response) {
  std::vector<journey> journeys;
  for (auto conn : *response->connections()) {
    journeys.emplace_back();
    auto& journey = journeys.back();

    /* stops */
    unsigned int stop_index = 0;
    for (auto stop : *conn->stops()) {
      journey.stops_.push_back(
          to_stop(*stop, stop_index++, conn->stops()->size()));
    }

    /* transports */
    for (auto move : *conn->transports()) {
      if (move->move_type() == Move_Walk) {
        auto walk = reinterpret_cast<Walk const*>(move->move());
        journey.transports_.push_back(to_transport(
            *walk, get_move_duration(*walk->range(), *conn->stops())));
      } else if (move->move_type() == Move_Transport) {
        auto transport = reinterpret_cast<Transport const*>(move->move());
        journey.transports_.push_back(to_transport(
            *transport,
            get_move_duration(*transport->range(), *conn->stops())));
      }
    }

    /* attributes */
    for (auto attribute : *conn->attributes()) {
      journey.attributes_.push_back(to_attribute(*attribute));
    }

    journey.duration_ = get_duration(journey);
    journey.transfers_ = get_transfers(journey);
  }
  return journeys;
}

}  // namespace motis
