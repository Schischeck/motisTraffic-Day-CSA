#pragma once

#include <memory>
#include <vector>

#include "motis/core/schedule/station.h"

namespace motis {
namespace intermodal {

class station_geo_index {
public:
  explicit station_geo_index(const std::vector<station_ptr>& stations);
  ~station_geo_index();

  std::vector<const station*> stations(double lat, double lng,
                                       double radius) const;

private:
  struct impl;
  std::unique_ptr<impl> impl_;
};

}  // namespace intermodal
}  // namespace motis
