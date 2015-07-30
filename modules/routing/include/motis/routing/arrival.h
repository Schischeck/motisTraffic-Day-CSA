#pragma once

#include <vector>
#include <array>

namespace motis {

struct arrival_part {
  arrival_part(int station)
      : station(station), time_cost(0), price(0), slot(0) {}
  arrival_part() : station(0), time_cost(0), price(0), slot(0) {}

  int station;
  int time_cost;
  int price;
  int slot;

  friend bool operator==(arrival_part const& a, arrival_part const& b) {
    return a.station == b.station && a.time_cost == b.time_cost &&
           a.price == b.price && a.slot == b.slot;
  }
};

typedef std::vector<arrival_part> arrival;

}  // namespace motis
