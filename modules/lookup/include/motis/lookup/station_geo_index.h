#pragma once

#include <memory>
#include <vector>

#include "motis/core/schedule/station.h"

#include "motis/protocol/Message_generated.h"

namespace motis {
namespace lookup {

class station_geo_index {
public:
  explicit station_geo_index(const std::vector<station_ptr>& stations);
  ~station_geo_index();

  std::vector<const station*> stations(double lat, double lng,
                                       double radius) const;

  flatbuffers::Offset<LookupGeoStationResponse> stations(
      flatbuffers::FlatBufferBuilder&, LookupGeoStationRequest const*) const;

private:
  struct impl;
  std::unique_ptr<impl> impl_;
};

}  // namespace lookup
}  // namespace motis
