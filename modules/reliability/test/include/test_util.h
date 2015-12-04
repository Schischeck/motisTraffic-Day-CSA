#pragma once

#include "motis/core/schedule/time.h"

namespace motis {
namespace reliability {
namespace test_util {
inline motis::time minutes_to_motis_time(uint16_t minutes) {
  return static_cast<motis::time>(minutes + 1440);
}
}
}
}
