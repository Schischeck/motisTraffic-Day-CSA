#pragma once

#include <string>
#include <vector>

namespace motis {
namespace reliability {
namespace intermodal {

struct hotel {
  explicit hotel(std::string const st, uint16_t const earliest_checkout,
                 uint16_t const min_stay_duration, uint16_t const price)
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
void parse_hotels(std::string const file_path, std::vector<hotel>&,
                  uint16_t const earliest_checkout,
                  uint16_t const min_stay_duration, uint16_t const price);

}  // namespace intermodal
}  // namespace reliability
}  // namespace motis
