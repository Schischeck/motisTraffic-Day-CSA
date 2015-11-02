#pragma once

#include <string>
#include <vector>

#include "motis/core/common/journey.h"

#include "motis/reliability/probability_distribution.h"
#include "motis/reliability/rating/connection_rating.h"

namespace motis {
namespace reliability {
namespace search {

struct connection_graph {
  struct stop {
    enum indices {
      Index_departure_stop = 0, /* source of the query */
      Index_arrival_stop = 1, /* destination of the query */
      Index_first_intermediate_stop = 2 /* first intermediate stop in the
                                           connection graph (note: all stops
                                           with index >= 2 are intermediate
                                           stops */
    };
    unsigned short index;

    struct departure_info {
      unsigned short departing_journey_index;
      unsigned short head_stop_index;
    };
    std::vector<departure_info> departure_infos;
  };

  struct journey_info {
    journey j;
    rating::connection_rating rating;
  };

  std::vector<stop> stops;
  std::vector<journey_info> journeys;

  std::pair<std::string, std::string> station_info(
      unsigned int const stop_idx) const {
    if (stop_idx == stop::Index_arrival_stop) {
      return arrival_station_info();
    }
    auto const& stop = journeys[stops.at(stop_idx)
                                    .departure_infos.front()
                                    .departing_journey_index].j.stops.front();
    return std::make_pair(stop.name, stop.eva_no);
  }

private:
  std::pair<std::string, std::string> arrival_station_info() const {
    auto it = std::find_if(stops.begin(), stops.end(), [](stop const& s) {
      return !s.departure_infos.empty() &&
             s.departure_infos.front().head_stop_index ==
                 stop::Index_arrival_stop;
    });
    auto const& stop =
        journeys.at(it->departure_infos.front().departing_journey_index)
            .j.stops.back();
    return std::make_pair(stop.name, stop.eva_no);
  }
};

}  // namespace search
}  // namespace reliability
}  // namespace motis
