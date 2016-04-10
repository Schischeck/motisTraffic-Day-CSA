#pragma once

#include <array>
#include <vector>

namespace motis {

struct arrival_part {
  explicit arrival_part(int station)
      : station_(station), time_cost_(0), price_(0), slot_(0) {}
  arrival_part() : station_(0), time_cost_(0), price_(0), slot_(0) {}

  friend bool operator==(arrival_part const& a, arrival_part const& b) {
    return a.station_ == b.station_ && a.time_cost_ == b.time_cost_ &&
           a.price_ == b.price_ && a.slot_ == b.slot_;
  }

  int station_;
  int time_cost_;
  int price_;
  int slot_;
};

using arrival = std::vector<arrival_part>;

}  // namespace motis
