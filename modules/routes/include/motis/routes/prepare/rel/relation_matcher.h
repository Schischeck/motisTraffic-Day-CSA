#pragma once

#include <vector>

#include "motis/geo/point_rtree.h"
#include "motis/geo/polyline.h"
#include "motis/geo/latlng.h"

#include "motis/routes/prepare/station_sequences.h"

namespace motis {
namespace routes {

struct match_seq {
  bool full_match_ = false;
  geo::polyline polyline_;
  std::vector<std::pair<size_t, size_t>> stations_;
};

std::vector<std::vector<match_seq>> match_osm_relations(
    std::string const& osm_file, std::vector<station_seq> const& sequences,
    std::map<std::string, std::vector<geo::latlng>> const& additional_stop_positions);

std::vector<std::vector<match_seq>> match_sequences(
    std::vector<geo::polyline> const& polylines,
    std::vector<station_seq> const& sequences,
    std::map<std::string, std::vector<geo::latlng>> const& bus_stops);

std::vector<match_seq> matches_on_seq(
    station_seq const& seq, geo::point_rtree const& rtree,
    geo::polyline const& polyline,
    std::map<std::string, std::vector<geo::latlng>> const& bus_stops, int const& radius);

void try_add_match(long front, long back, geo::polyline const& polyline,
                   std::vector<std::pair<size_t, size_t>>& stations,
                   bool full_match, std::vector<match_seq>& matches);

}  // namespace routes
}  // namespace motis
