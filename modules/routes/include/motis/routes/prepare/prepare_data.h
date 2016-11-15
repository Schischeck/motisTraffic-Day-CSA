#pragma once

#include <map>
#include <string>
#include <vector>

#include "geo/polygon.h"

namespace motis {
namespace routes {

struct kv_database;
struct station_seq;
struct routing_strategy;

void prepare(
    std::vector<station_seq>& sequences,
    std::map<std::string, std::vector<geo::latlng>> const& stop_positions,
    std::string const& osm, kv_database& db);

}  // namespace routes
}  // namespace motis