#ifndef TD_ARRIVAL_H_
#define TD_ARRIVAL_H_

#include <vector>
#include <array>

namespace td {

struct ArrivalPart
{
  ArrivalPart()
      : station(0),
        timeCost(0),
        price(0),
        slot(0)
  {}

  int station;
  int timeCost;
  int price;
  int slot;

  friend bool operator==(ArrivalPart const& a, ArrivalPart const& b) {
    return
      a.station == b.station &&
      a.timeCost == b.timeCost &&
      a.price == b.price &&
      a.slot == b.slot;
  }
};

typedef std::vector<ArrivalPart> Arrival;

}  // namespace td

#endif  // TD_ARRIVAL_H_
