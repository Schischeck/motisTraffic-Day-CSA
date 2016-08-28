#include "motis/routes/prepare/station_sequences.h"

#include <vector>

#include "motis/routes/prepare/fbs/use_64bit_flatbuffers.h"

#include "motis/core/common/geo.h"

#include "motis/schedule-format/Schedule_generated.h"

namespace motis {
namespace routes {

std::vector<station_seq> load_station_sequences(
    motis::loader::Schedule const* sched) {
  std::vector<station_seq> result;

  for (auto const& route : *sched->routes()) {
    station_seq seq;

    for (auto const& station : *route->stations()) {
      seq.station_ids_.emplace_back(station->id()->str());
      seq.coordinates_.emplace_back(station->lat(), station->lng());
    }
    result.emplace_back(std::move(seq));
  }

  std::sort(begin(result), end(result));
  result.erase(std::unique(begin(result), end(result)), end(result));

  return result;
}

}  // namespace routes
}  // namespace motis
