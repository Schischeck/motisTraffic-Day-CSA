#include "motis/core/common/journey_builder.h"

#include <algorithm>

#include "motis/core/common/journey.h"

#include "motis/protocol/RoutingResponse_generated.h"

namespace motis {

journey::stop::event_info to_event_info(routing::EventInfo const* event,
                                        bool const valid) {
  journey::stop::event_info e;
  e.platform = event->platform()->c_str();
  e.timestamp = event->time();
  e.valid = valid;
  return e;
}

journey::stop to_stop(routing::Stop const* stop, unsigned int const index,
                      unsigned int const num_stops) {
  journey::stop s;
  s.index = index;
  s.eva_no = stop->eva_nr()->c_str();
  s.interchange = (bool)stop->interchange();
  s.lat = 0.0;
  s.lng = 0.0;
  s.name = stop->name()->c_str();
  s.arrival = to_event_info(stop->arrival(), index != 0);
  s.departure = to_event_info(stop->departure(), index + 1 != num_stops);
  return s;
}
journey::transport to_transport(routing::MoveWrapper const* move) {
  journey::transport t;
  return t;
}
journey::attribute to_attribute(routing::Attribute const* attribute) {
  return journey::attribute();
}

int get_duration(journey const& journey) {
  if (journey.stops.size() > 0) {
    return (journey.stops.back().arrival.timestamp -
            journey.stops.front().departure.timestamp) /
           60;
  }
  return 0;
}

int get_transfers(journey const& journey) {
  return std::count_if(journey.stops.begin(), journey.stops.end(),
                       [](journey::stop const& s) { return s.interchange; });
}

std::vector<journey> to_journeys(routing::RoutingResponse const* response) {
  std::vector<journey> journeys;
  for (auto conn = response->connections()->begin();
       conn != response->connections()->end(); ++conn) {
    journeys.emplace_back();
    auto& journey = journeys.back();
    unsigned int stop_index = 0;
    for (auto stop = conn->stops()->begin(); stop != conn->stops()->end();
         ++stop, ++stop_index) {
      journey.stops.push_back(
          to_stop(*stop, stop_index, conn->stops()->size()));
    }
    for (auto transport = conn->transports()->begin();
         transport != conn->transports()->end(); ++transport) {
      journey.transports.push_back(to_transport(*transport));
    }
    for (auto attribute = conn->attributes()->begin();
         attribute != conn->attributes()->end(); ++attribute) {
      journey.attributes.push_back(to_attribute(*attribute));
    }

    journey.duration = get_duration(journey);
    journey.transfers = get_transfers(journey);
  }
  return journeys;
}

}  // namespace motis
