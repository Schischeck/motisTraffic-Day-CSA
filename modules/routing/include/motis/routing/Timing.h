#pragma once

#include <chrono>

#define MOTIS_START_TIMING(_X) \
  auto _X##_start = std::chrono::steady_clock::now(), _X##_stop = _X##_start
#define MOTIS_STOP_TIMING(_X) _X##_stop = std::chrono::steady_clock::now()
#define MOTIS_TIMING_MS(_X)                                          \
  (std::chrono::duration_cast<std::chrono::milliseconds>(_X##_stop - \
                                                         _X##_start).count())
