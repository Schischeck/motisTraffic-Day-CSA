#pragma once

#include <string>

#include "motis/module/message.h"

namespace motis {
struct schedule;
namespace reliability {
struct ReliableRoutingRequest;  // NOLINT
namespace search {
namespace late_connections {
namespace detail {
struct taxi_cost {
  taxi_cost(double const& lat1, double const& lon1, double const& lat2,
            double const& lon2, unsigned const taxi_base_price,
            unsigned const taxi_km_price, unsigned const taxi_base_time,
            unsigned const taxi_avg_speed_short_distance,
            unsigned const taxi_avg_speed_long_distance);
  uint16_t duration_, price_;
};
}  // namespace detail

module::msg_ptr search(ReliableRoutingRequest const&,
                       std::string const& hotels_file, schedule const&);
}  // namespace late_connections
}  // namespace search
}  // namespace reliability
}  // namespace motis
