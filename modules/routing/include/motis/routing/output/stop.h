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
      : index(index),
        station_id(station_id),
        a_platform(a_platform),
        d_platform(d_platform),
        a_time(a_time),
        d_time(d_time),
        interchange(interchange) {}

  unsigned int index;
  unsigned int station_id;
  unsigned int a_platform, d_platform;
  time a_time, d_time;
  bool interchange;
};

}  // namespace intermediate
}  // namespace output
}  // namespace routing
}  // namespace motis