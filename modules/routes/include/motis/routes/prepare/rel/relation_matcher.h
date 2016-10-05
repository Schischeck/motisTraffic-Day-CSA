#pragma once

#include <map>
#include <vector>

#include "geo/latlng.h"
#include "geo/polyline.h"

#include "motis/routes/prepare/source_spec.h"
#include "motis/routes/prepare/station_sequences.h"

namespace motis {
namespace routes {

struct match_seq {
  source_spec source_;
  geo::polyline polyline_;
  std::vector<std::pair<size_t, size_t>> stations_;
};

std::vector<std::vector<match_seq>> match_osm_relations(
    std::string const& osm_file, std::vector<station_seq> const& sequences,
    std::map<std::string, std::vector<geo::latlng>> const&
        additional_stop_positions);

}  // namespace routes
}  // namespace motis
