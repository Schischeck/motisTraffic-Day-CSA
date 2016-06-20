#pragma once

#include <memory>
#include <vector>

#include "motis/core/schedule/station.h"

#include "motis/protocol/Message_generated.h"

namespace motis {
namespace lookup {

class station_geo_index {
public:
  explicit station_geo_index(std::vector<station_ptr> const& stations);
  ~station_geo_index();

  std::vector<station const*> stations(double lat, double lng,
                                       double min_radius,
                                       double max_radius) const;

private:
  struct impl;
  std::unique_ptr<impl> impl_;
};

}  // namespace lookup
}  // namespace motis
