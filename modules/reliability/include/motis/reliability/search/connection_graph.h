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
    unsigned short index_;

    struct departure_info {
      unsigned short departing_journey_index_;
      unsigned short head_stop_index_;
    };
    std::vector<departure_info> departure_infos_;
  };

  struct journey_info {
    journey j_;
    rating::connection_rating rating_;
  };

  std::vector<stop> stops_;
  std::vector<journey_info> journeys_;

  std::pair<std::string, std::string> station_info(
      unsigned int const stop_idx) const {
    if (stop_idx == stop::Index_arrival_stop) {
      return arrival_station_info();
    }
    auto const& stop = journeys_[stops_.at(stop_idx)
                                    .departure_infos_.front()
                                    .departing_journey_index_].j_.stops.front();
    return std::make_pair(stop.name, stop.eva_no);
  }

private:
  std::pair<std::string, std::string> arrival_station_info() const {
    auto it = std::find_if(stops_.begin(), stops_.end(), [](stop const& s) {
      return !s.departure_infos_.empty() &&
             s.departure_infos_.front().head_stop_index_ ==
                 stop::Index_arrival_stop;
    });
    auto const& stop =
        journeys_.at(it->departure_infos_.front().departing_journey_index_)
            .j_.stops.back();
    return std::make_pair(stop.name, stop.eva_no);
  }
};

}  // namespace search
}  // namespace reliability
}  // namespace motis
