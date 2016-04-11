#pragma once

#include "motis/core/schedule/time.h"

namespace motis {
namespace routing {
namespace output {
namespace intermediate {

struct stop {
  stop() = default;
  stop(unsigned int index, unsigned int station_id, unsigned int a_platform,
       unsigned int d_platform, time a_time, time d_time, bool interchange)
      : index_(index),
        station_id_(station_id),
        a_platform_(a_platform),
        d_platform_(d_platform),
        a_time_(a_time),
        d_time_(d_time),
        interchange_(interchange) {}

  unsigned int index_;
  unsigned int station_id_;
  unsigned int a_platform_, d_platform_;
  time a_time_, d_time_;
  bool interchange_;
};

}  // namespace intermediate
}  // namespace output
}  // namespace routing
}  // namespace motis