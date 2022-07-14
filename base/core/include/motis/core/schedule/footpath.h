#pragma once

#include "motis/core/schedule/time.h"

#include <tuple>

namespace motis {

struct footpath {

  footpath(uint32_t const& from_s, uint32_t const& to_s, time const& duration)
      : from_station_(from_s), to_station_(to_s), duration_(duration) {}

  bool operator==(footpath const& rhs) const {
    return std::tie(this->from_station_, this->to_station_, this->duration_) ==
           std::tie(rhs.from_station_, rhs.to_station_, rhs.duration_);
  }

  bool operator!=(footpath const& rhs) const {
    return std::tie(this->from_station_, this->to_station_, this->duration_) !=
           std::tie(rhs.from_station_, rhs.to_station_, rhs.duration_);
  }

  bool operator<(footpath const& rhs) const {
    return std::tie(this->from_station_, this->to_station_, this->duration_) <
           std::tie(rhs.from_station_, rhs.to_station_, rhs.duration_);
  }

  bool operator<=(footpath const& rhs) const {
    return std::tie(this->from_station_, this->to_station_, this->duration_) <=
           std::tie(rhs.from_station_, rhs.to_station_, rhs.duration_);
  }

  bool operator>(footpath const& rhs) const {
    return std::tie(this->from_station_, this->to_station_, this->duration_) >
           std::tie(rhs.from_station_, rhs.to_station_, rhs.duration_);
  }

  bool operator>=(footpath const& rhs) const {
    return std::tie(this->from_station_, this->to_station_, this->duration_) >=
           std::tie(rhs.from_station_, rhs.to_station_, rhs.duration_);
  }
  uint32_t from_station_;
  uint32_t to_station_;
  time duration_;
};

}  // namespace motis
