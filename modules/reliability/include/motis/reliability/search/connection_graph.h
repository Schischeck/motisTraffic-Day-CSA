#pragma once

#include <string>
#include <vector>

#include "motis/core/journey/journey.h"

#include "motis/reliability/distributions/probability_distribution.h"
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

    struct alternative_info {
      unsigned short journey_index_;
      unsigned short next_stop_index_;

      struct rating {
        probability_distribution departure_distribution_;
        probability_distribution arrival_distribution_;
      } rating_;
    };
    std::vector<alternative_info> alternative_infos_;
  };

  std::vector<stop> stops_;
  std::vector<journey> journeys_;

  std::pair<std::string, std::string> station_info(
      unsigned int const stop_idx) const {
    if (stop_idx == stop::Index_arrival_stop) {
      return arrival_station_info();
    }
    auto const& stop =
        journeys_[stops_.at(stop_idx).alternative_infos_.front().journey_index_]
            .stops.front();
    return std::make_pair(stop.name, stop.eva_no);
  }

private:
  std::pair<std::string, std::string> arrival_station_info() const {
    auto it = std::find_if(stops_.begin(), stops_.end(), [](stop const& s) {
      return !s.alternative_infos_.empty() &&
             s.alternative_infos_.front().next_stop_index_ ==
                 stop::Index_arrival_stop;
    });
    auto const& stop =
        journeys_.at(it->alternative_infos_.front().journey_index_)
            .stops.back();
    return std::make_pair(stop.name, stop.eva_no);
  }
};

}  // namespace search
}  // namespace reliability
}  // namespace motis
