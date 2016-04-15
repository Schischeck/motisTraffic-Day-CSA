#pragma once

#include <algorithm>
#include <array>
#include <vector>

namespace motis {

struct arrival_part {
  constexpr static uint16_t ALWAYS_VALID = 0;

  arrival_part(int station)
      : station(station),
        time_cost(0),
        price(0),
        slot(0),
        valid_from(ALWAYS_VALID),
        periodical_validity(false) {}
  arrival_part()
      : station(0),
        time_cost(0),
        price(0),
        slot(0),
        valid_from(ALWAYS_VALID),
        periodical_validity(false) {}

  int station;
  int time_cost;
  int price;
  int slot;
  uint16_t valid_from;
  bool periodical_validity;

  time interval_start(time const q_start) const {
    if (valid_from == ALWAYS_VALID) {
      return q_start;
    }
    return std::max(
        static_cast<uint16_t>(periodical_validity ? q_start % 1440 : q_start),
        valid_from);
  }

  friend bool operator==(arrival_part const& a, arrival_part const& b) {
    return a.station == b.station && a.time_cost == b.time_cost &&
           a.price == b.price && a.slot == b.slot;
  }
};

typedef std::vector<arrival_part> arrival;

}  // namespace motis
