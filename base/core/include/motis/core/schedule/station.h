#pragma once

#include <cmath>
#include <array>
#include <memory>
#include <string>

#include "motis/core/schedule/timezone.h"

namespace motis {

class station {
public:
  station(int index, double length, double width, int transfer_time,
          std::string eva_nr, std::string name, timezone const* timez)
      : index_(index),
        length_(length),
        width_(width),
        transfer_time_(transfer_time),
        arr_class_events_({{0}}),
        dep_class_events_({{0}}),
        eva_nr_(std::move(eva_nr)),
        name_(std::move(name)),
        timez_(timez) {}

  station()
      : index_(0),
        length_(0.0),
        width_(0.0),
        transfer_time_(0),
        arr_class_events_({{0}}),
        dep_class_events_({{0}}),
        timez_(nullptr) {}

  double lat() const { return width_; }
  double lng() const { return length_; }

  unsigned index_;
  double length_, width_;
  int transfer_time_;
  std::array<uint64_t, 10> arr_class_events_, dep_class_events_;
  std::string eva_nr_, name_;
  timezone const* timez_;
  std::vector<station*> equivalent_;
};

typedef std::unique_ptr<station> station_ptr;

}  // namespace motis
