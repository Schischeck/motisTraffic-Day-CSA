#pragma once

#include <string>
#include <vector>

namespace motis {
namespace reliability {
namespace intermodal {
namespace hotels {
struct hotel_info {
  explicit hotel_info(std::string const st, uint16_t earliest_checkout = 8 * 60,
                      uint16_t min_stay_duration = 9 * 60,
                      uint16_t price = 5000)
      : station_(st),
        earliest_checkout_(earliest_checkout),
        min_stay_duration_(min_stay_duration),
        price_(price) {}
  std::string station_;
  uint16_t earliest_checkout_;
  uint16_t min_stay_duration_;
  uint16_t price_;
};

/* get the eva numbers of all stations at which hotels are located */
std::vector<hotel_info> parse_hotels(std::string const file_path);

}  // namespace hotels
}  // namespace intermodal
}  // namespace reliability
}  // namespace motis
