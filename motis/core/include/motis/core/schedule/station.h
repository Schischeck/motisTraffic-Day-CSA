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
        arr_class_events({{0}}),
        dep_class_events({{0}}),
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
  std::array<uint64_t, 10> arr_class_events, dep_class_events;
  std::string eva_nr, name;
};

typedef std::unique_ptr<station> station_ptr;

}  // namespace motis
