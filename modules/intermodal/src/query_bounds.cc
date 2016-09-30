#include "motis/intermodal/query_bounds.h"

#include "motis/intermodal/error.h"

using namespace flatbuffers;
using namespace motis::routing;

namespace motis {
namespace intermodal {

query_start parse_query_start(FlatBufferBuilder& fbb,
                              IntermodalRoutingRequest const* req) {
  auto start_station = CreateInputStation(fbb, fbb.CreateString(STATION_START),
                                          fbb.CreateString(STATION_START));
  switch (req->start_type()) {
    case IntermodalStart_IntermodalOntripStart: {
      auto start = reinterpret_cast<IntermodalOntripStart const*>(req->start());
      return {
          Start_OntripStationStart,
          CreateOntripStationStart(fbb, start_station, start->departure_time())
              .Union(),
          {start->position()->lat(), start->position()->lng()}};
    } break;
    case IntermodalStart_IntermodalPretripStart: {
      auto start =
          reinterpret_cast<IntermodalPretripStart const*>(req->start());
      return {Start_PretripStart,
              CreatePretripStart(fbb, start_station, start->interval()).Union(),
              {start->position()->lat(), start->position()->lng()}};
    } break;
    case IntermodalStart_OntripTrainStart: {
      // TODO
    } break;
    case IntermodalStart_OntripStationStart: {
      // TODO
    } break;
    case IntermodalStart_PretripStart: {
      // TODO
    } break;
    default: throw std::system_error(error::unknown_start);
  }
}

query_dest parse_query_dest(FlatBufferBuilder& fbb,
                            IntermodalRoutingRequest const* req) {
  switch (req->destination_type()) {
    case IntermodalDestination_InputStation: {
      // TODO
    } break;
    case IntermodalDestination_InputPosition: {
      auto pos = reinterpret_cast<InputPosition const*>(req->destination());
      auto end_station = CreateInputStation(fbb, fbb.CreateString(STATION_END),
                                            fbb.CreateString(STATION_END));
      return {end_station, {pos->lat(), pos->lng()}};

    } break;
    default: throw std::system_error(error::unknown_destination);
  }
}

}  // namespace intermodal
}  // namespace motis
