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
      Index_arrival_stop = 1 /* destination of the query */
    };
    struct interchange_info {
      unsigned short departing_journey_index;
    };

    unsigned short index;
    std::string name;
    std::string eva_no;
    std::vector<interchange_info> interchange_infos;
  };
  struct journey_info {
    unsigned short from_index;
    unsigned short to_index;
    journey j;
    rating::connection_rating rating;
  };

  std::vector<stop> stops;
  std::vector<journey_info> journeys;
};

}  // namespace search
}  // namespace reliability
}  // namespace motis
