#include "motis/intermodal/query_bounds.h"

#include "parser/util.h"

#include "motis/module/message.h"

#include "motis/intermodal/error.h"

using namespace flatbuffers;
using namespace motis::routing;
using namespace motis::module;

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
      break;
    }

    case IntermodalStart_IntermodalPretripStart: {
      auto start =
          reinterpret_cast<IntermodalPretripStart const*>(req->start());
      return {Start_PretripStart,
              CreatePretripStart(fbb, start_station, start->interval()).Union(),
              {start->position()->lat(), start->position()->lng()}};
      break;
    }

    case IntermodalStart_OntripTrainStart:
      return {Start_OntripTrainStart,
              motis_copy_table(OntripTrainStart, fbb, req->start()).Union()};

    case IntermodalStart_OntripStationStart:
      return {Start_OntripStationStart,
              motis_copy_table(OntripStationStart, fbb, req->start()).Union()};

    case IntermodalStart_PretripStart:
      return {Start_PretripStart,
              motis_copy_table(PretripStart, fbb, req->start()).Union()};

    default: verify(false, "invalid query start");
  }
}

query_dest parse_query_dest(FlatBufferBuilder& fbb,
                            IntermodalRoutingRequest const* req) {
  switch (req->destination_type()) {
    case IntermodalDestination_InputStation:
      return query_dest{
          motis_copy_table(InputStation, fbb, req->destination())};

    case IntermodalDestination_InputPosition: {
      auto pos = reinterpret_cast<InputPosition const*>(req->destination());
      auto end_station = CreateInputStation(fbb, fbb.CreateString(STATION_END),
                                            fbb.CreateString(STATION_END));
      return {end_station, {pos->lat(), pos->lng()}};
      break;
    }

    default: verify(false, "invalid query dest");
  }
}

}  // namespace intermodal
}  // namespace motis
