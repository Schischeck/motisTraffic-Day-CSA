#pragma once

#include "motis/core/schedule/time.h"

namespace motis {
namespace routing {
namespace output {
namespace intermediate {

struct stop {
  stop() = default;
  stop(unsigned int index, unsigned int station_id, unsigned int a_track,
       unsigned int d_track, time a_time, time d_time, time a_sched_time,
       time d_sched_time, timestamp_reason a_reason, timestamp_reason d_reason,
       bool interchange)
      : index_(index),
        station_id_(station_id),
        a_track_(a_track),
        d_track_(d_track),
        a_time_(a_time),
        d_time_(d_time),
        a_sched_time_(a_sched_time),
        d_sched_time_(d_sched_time),
        a_reason_(a_reason),
        d_reason_(d_reason),
        interchange_(interchange) {}

  unsigned int index_;
  unsigned int station_id_;
  unsigned int a_track_, d_track_;
  time a_time_, d_time_;
  time a_sched_time_, d_sched_time_;
  timestamp_reason a_reason_, d_reason_;
  bool interchange_;
};

}  // namespace intermediate
}  // namespace output
}  // namespace routing
}  // namespace motis
