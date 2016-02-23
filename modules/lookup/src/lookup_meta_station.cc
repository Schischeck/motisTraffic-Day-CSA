#include "motis/lookup/lookup_meta_station.h"

#include "motis/lookup/util.h"

using namespace flatbuffers;

namespace motis {
namespace lookup {

Offset<LookupMetaStationResponse> lookup_meta_station(
    FlatBufferBuilder& fbb, schedule const& sched,
    LookupMetaStationRequest const* req) {
  std::vector<Offset<Station>> equivalent;

  auto station = get_station(sched, req->eva_nr()->str());
  for (auto const& e : station->equivalent) {
    equivalent.push_back(create_station(fbb, *e));
  }

  return CreateLookupMetaStationResponse(fbb, fbb.CreateVector(equivalent));
}

}  // namespace lookup
}  // namespace motis
