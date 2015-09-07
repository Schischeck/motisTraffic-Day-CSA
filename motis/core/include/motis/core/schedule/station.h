#pragma once

#include <memory>
#include <cmath>
#include <string>
#include <array>

namespace motis {

class station {
public:
  station(int index, double length, double width, int transfer_time,
          std::string eva_nr, std::string name)
      : index(index),
        length(length),
        width(width),
        transfer_time(transfer_time),
        eva_nr(std::move(eva_nr)),
        name(std::move(name)) {}

  station()
      : index(0),
        length(0.0),
        width(0.0),
        transfer_time(0),
        arr_class_events({{0}}),
        dep_class_events({{0}}) {}

  int index;
  double length, width;
  int transfer_time;
  std::array<int, 10> arr_class_events, dep_class_events;
  std::string eva_nr, name;

  // normally us_hoch should be equal to us_nieder
  // not quite sure how to handle that
  int get_transfer_time() const { return transfer_time; }
};

typedef std::unique_ptr<station> station_ptr;

}  // namespace motis
