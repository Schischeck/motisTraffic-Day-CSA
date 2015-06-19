#ifndef TD_ARRIVAL_H_
#define TD_ARRIVAL_H_

#include <vector>
#include <array>

namespace td {

struct arrival_part {
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

}  // namespace td

#endif  // TD_ARRIVAL_H_
