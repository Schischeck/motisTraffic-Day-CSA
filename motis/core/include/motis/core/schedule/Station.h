#ifndef TDSTATIONS_H
#define TDSTATIONS_H TDSTATIONS_H

#include <memory>
#include <cmath>
#include <string>
#include <array>

#include "motis/core/common/Array.h"
#include "motis/core/common/Deleter.h"

namespace td
{

class Station
{
public:
  Station()
    : index(0),
      length(0.0),
      width(0.0),
      usHoch(0),
      usNieder(0),
      arrClassEvents({ { 0 } }),
      depClassEvents({ { 0 } })
  {}

  int index;
  double length, width;
  int usHoch, usNieder;
  std::array<int, 10> arrClassEvents, depClassEvents;
  String evaNr;
  String name;

  //normally usHoch should be equal to usNieder
  //not quite sure how to handle that
  int getTransferTime() const { return std::max(usHoch, usNieder); }
};

typedef std::unique_ptr<Station, Deleter<Station>> StationPtr;

}  // namespace td

#endif //TDSTATIONS_H

