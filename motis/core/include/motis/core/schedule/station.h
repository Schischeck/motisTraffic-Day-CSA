#pragma once

#include <memory>
#include <cmath>
#include <string>
#include <array>

#include "motis/core/common/array.h"

namespace motis {

class station {
public:
  station()
      : index(0),
        length(0.0),
        width(0.0),
        us_hoch(0),
        us_nieder(0),
        arr_class_events({{0}}),
        dep_class_events({{0}}) {}

  int index;
  double length, width;
  int us_hoch, us_nieder;
  std::array<int, 10> arr_class_events, dep_class_events;
  int eva_nr;
  string name;

  // normally us_hoch should be equal to us_nieder
  // not quite sure how to handle that
  int get_transfer_time() const { return std::max(us_hoch, us_nieder); }
};

typedef std::unique_ptr<station> station_ptr;

}  // namespace motis
