#pragma once

#include <string>
#include <vector>

namespace motis {
namespace reliability {
namespace intermodal {

constexpr auto HOTEL_EARLIEST_CHECKOUT = 6 * 60;
constexpr auto HOTEL_MIN_STAY_DURATION = 9 * 60;
constexpr auto HOTEL_PRICE = 5000;

struct hotel {
  explicit hotel(std::string const st,
                 uint16_t const earliest_checkout = HOTEL_EARLIEST_CHECKOUT,
                 uint16_t const min_stay_duration = HOTEL_MIN_STAY_DURATION,
                 uint16_t const price = HOTEL_PRICE)
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
void parse_hotels(std::string const file_path, std::vector<hotel>&);

}  // namespace intermodal
}  // namespace reliability
}  // namespace motis
