#pragma once

#include <vector>

#include "motis/core/schedule/time.h"

#include "motis/reliability/computation/data_departure_interchange.h"
#include "motis/reliability/distributions/probability_distribution.h"

namespace motis {
namespace reliability {
namespace calc_departure_distribution {
namespace interchange {
void compute_departure_distribution(data_departure_interchange const&,
                                    probability_distribution&);

namespace detail {
probability ic_feeder_arrived(
    data_departure_interchange::interchange_feeder_info const&
        interchange_feeder_info,
    time const timestamp);

probability ic_feeder_arrives_at_time(
    data_departure_interchange::interchange_feeder_info const&
        interchange_feeder_info,
    time const timestamp);
}  // namespace detail
}  // namespace interchange
}  // namespace calc_departure_distribution
}  // namespace reliability
}  // namespace motis
