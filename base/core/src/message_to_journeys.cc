#include "motis/core/journey/message_to_journeys.h"

#include <algorithm>

#include "motis/core/journey/journey.h"
#include "motis/core/journey/journey_util.h"
#include "motis/core/schedule/category.h"

#include "motis/protocol/RoutingResponse_generated.h"

namespace motis {

journey::stop::event_info to_event_info(routing::EventInfo const& event,
                                        bool const valid) {
  journey::stop::event_info e;
  e.platform = event.platform()->c_str();
  e.timestamp = event.time();
  e.schedule_timestamp = event.schedule_time();
  e.valid = valid;
  return e;
}
journey::stop to_stop(routing::Stop const& stop, unsigned int const index,
                      unsigned int const num_stops) {
  journey::stop s;
  s.index = index;
  s.eva_no = stop.eva_nr()->c_str();
  s.interchange = (bool)stop.interchange();
  s.lat = stop.lat();
  s.lng = stop.lng();
  s.name = stop.name()->c_str();
  s.arrival = to_event_info(*stop.arrival(), index != 0);
  s.departure = to_event_info(*stop.departure(), index + 1 != num_stops);
  return s;
}
journey::transport create_empty_transport() {
  journey::transport t;
  t.category_id = 0;
  t.category_name = "";
  t.direction = "";
  t.from = 0;
  t.line_identifier = "";
  t.mumo_price = 0;
  t.mumo_type_name = "";
  t.name = "";
  t.provider = "";
  t.route_id = 0;
  t.slot = 0;
  t.to = 0;
  t.train_nr = 0;
  t.type = journey::transport::PublicTransport;
  return t;
}
journey::transport to_transport(routing::Walk const& walk, uint16_t duration) {
  auto t = create_empty_transport();
  t.type = journey::transport::Walk;
  t.duration = duration;
  t.from = walk.range()->from();
  t.to = walk.range()->to();
  return t;
}
journey::transport to_transport(routing::Transport const& transport,
                                uint16_t duration) {
  auto t = create_empty_transport();
  t.duration = duration;
  t.from = transport.range()->from();
  t.to = transport.range()->to();
  t.type = journey::transport::PublicTransport;
  t.category_name = transport.category_name()->c_str();
  t.category_id = transport.category_id();
  t.clasz = transport.clasz();
  t.direction = transport.direction()->c_str();
  t.line_identifier = transport.line_id()->c_str();
  t.name = transport.name()->c_str();
  t.provider = transport.provider()->c_str();
  t.route_id = transport.route_id();
  t.slot = 0;
  t.train_nr = transport.train_nr();
  return t;
}
journey::transport to_transport(routing::Mumo const& mumo, uint16_t duration) {
  auto t = create_empty_transport();
  t.type = journey::transport::Mumo;
  t.duration = duration;
  t.from = mumo.range()->from();
  t.to = mumo.range()->to();
  t.mumo_price = mumo.price();
  t.mumo_type_name = mumo.name()->c_str();
  return t;
}

journey::attribute to_attribute(routing::Attribute const& attribute) {
  journey::attribute a;
  a.code = attribute.code()->c_str();
  a.from = attribute.from();
  a.text = attribute.text()->c_str();
  a.to = attribute.to();
  return a;
}

uint16_t get_move_duration(
    routing::Range const& range,
    flatbuffers::Vector<flatbuffers::Offset<routing::Stop>> const& stops) {
  routing::Stop const& from = *stops[range.from()];
  routing::Stop const& to = *stops[range.to()];
  return (to.arrival()->time() - from.departure()->time()) / 60;
}

std::vector<journey> message_to_journeys(
    routing::RoutingResponse const* response) {
  std::vector<journey> journeys;
  for (auto conn = response->connections()->begin();
       conn != response->connections()->end(); ++conn) {
    journeys.emplace_back();
    auto& journey = journeys.back();

    /* stops */
    unsigned int stop_index = 0;
    for (auto stop = conn->stops()->begin(); stop != conn->stops()->end();
         ++stop, ++stop_index) {
      journey.stops.push_back(
          to_stop(**stop, stop_index, conn->stops()->size()));
    }

    /* transports */
    for (auto transport = conn->transports()->begin();
         transport != conn->transports()->end(); ++transport) {
      auto const move = *transport;
      switch (move->move_type()) {
        case routing::Move_Walk: {
          auto walk = (routing::Walk const*)move->move();
          journey.transports.push_back(to_transport(
              *walk, get_move_duration(*walk->range(), *conn->stops())));
          break;
        }
        case routing::Move_Transport: {
          auto transport = (routing::Transport const*)move->move();
          journey.transports.push_back(to_transport(
              *transport,
              get_move_duration(*transport->range(), *conn->stops())));
          break;
        }
        case routing::Move_Mumo: {
          auto mumo = (routing::Mumo const*)move->move();
          journey.transports.push_back(to_transport(
              *mumo, get_move_duration(*mumo->range(), *conn->stops())));
          break;
        }
        default: break;
      }
    }

    /* attributes */
    for (auto attribute = conn->attributes()->begin();
         attribute != conn->attributes()->end(); ++attribute) {
      journey.attributes.push_back(to_attribute(**attribute));
    }

    journey.duration = get_duration(journey);
    journey.transfers = get_transfers(journey);
  }
  return journeys;
}

}  // namespace motis
