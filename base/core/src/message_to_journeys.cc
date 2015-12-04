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
  s.lat = 0.0;
  s.lng = 0.0;
  s.name = stop.name()->c_str();
  s.arrival = to_event_info(*stop.arrival(), index != 0);
  s.departure = to_event_info(*stop.departure(), index + 1 != num_stops);
  return s;
}
journey::transport to_transport(routing::Walk const& walk, uint16_t duration) {
  journey::transport t;
  t.duration = duration;
  t.from = walk.range()->from();
  t.to = walk.range()->to();
  t.walk = true;
  t.category_id = 0;
  t.category_name = "";
  t.direction = "";
  t.line_identifier = "";
  t.name = "";
  t.provider = "";
  t.slot = 0;
  t.train_nr = 0;
  t.route_id = 0;
  return t;
}
journey::transport to_transport(routing::Transport const& transport,
                                uint16_t duration) {
  journey::transport t;
  t.category_name = transport.category_name()->c_str();
  t.category_id = transport.category_id();
  t.direction = transport.direction()->c_str();
  t.duration = duration;
  t.from = transport.range()->from();
  t.line_identifier = transport.line_id()->c_str();
  t.name = transport.name()->c_str();
  t.provider = transport.provider()->c_str();
  t.route_id = transport.route_id();
  t.slot = 0;
  t.to = transport.range()->to();
  t.train_nr = transport.train_nr();
  t.walk = false;
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

std::vector<journey> message_to_journeys(routing::RoutingResponse const* response) {
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
      if (move->move_type() == routing::Move_Walk) {
        auto walk = (routing::Walk const*)move->move();
        journey.transports.push_back(to_transport(
            *walk, get_move_duration(*walk->range(), *conn->stops())));
      } else if (move->move_type() == routing::Move_Transport) {
        auto transport = (routing::Transport const*)move->move();
        journey.transports.push_back(to_transport(
            *transport,
            get_move_duration(*transport->range(), *conn->stops())));
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
