#pragma once

#include "motis/core/schedule/time.h"

namespace motis {
namespace reliability {
namespace test_util {

inline motis::time minutes_to_motis_time(uint16_t minutes) {
  return to_motis_time(minutes);
}

}  // namespace test_util
}  // namespace reliability
}  // namespace motis
