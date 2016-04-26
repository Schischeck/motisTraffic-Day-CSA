#include "motis/lookup/lookup_geo_station.h"

#include "motis/core/access/station_access.h"
#include "motis/lookup/util.h"

using namespace flatbuffers;

namespace motis {
namespace lookup {

Offset<LookupGeoStationResponse> lookup_geo_stations_id(
    FlatBufferBuilder& b, station_geo_index const& idx, schedule const& sched,
    LookupGeoStationIdRequest const* req) {
  auto const& station = get_station(sched, req->station_id()->str());
  std::vector<Offset<Station>> list;
  for (auto const& s :
       idx.stations(station->lat(), station->lng(), req->radius())) {
    list.push_back(create_station(b, *s));
  }
  return CreateLookupGeoStationResponse(b, b.CreateVector(list));
}

Offset<LookupGeoStationResponse> lookup_geo_stations(
    FlatBufferBuilder& b, station_geo_index const& idx,
    LookupGeoStationRequest const* req) {
  std::vector<Offset<Station>> list;
  for (auto const& s : idx.stations(req->lat(), req->lng(), req->radius())) {
    list.push_back(create_station(b, *s));
  }
  return CreateLookupGeoStationResponse(b, b.CreateVector(list));
}

}  // namespace lookup
}  // namespace motis
