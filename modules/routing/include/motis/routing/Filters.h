#ifndef TD_FILTERS_H
#define TD_FILTERS_H TD_FILTERS_H

#include <algorithm>

#include "motis/core/schedule/connection.h"

#define MAX_TRAVEL_TIME (1440)
#define MAX_TRANSFERS (6)
#define MAX_WAITING_TIME (200)

namespace td {

inline bool is_filtered_travel_time(unsigned const n_travel_time) {
  return n_travel_time > MAX_TRAVEL_TIME;
}

inline bool is_filtered_transfers(unsigned const n_transfers) {
  return n_transfers > MAX_TRANSFERS;
}

inline bool is_filtered_waiting_time(light_connection const* con,
                                     unsigned pred_travel_time,
                                     unsigned next_travel_time) {
  unsigned con_time = con != nullptr ? con->a_time - con->d_time : 0;
  return next_travel_time - pred_travel_time - con_time > MAX_WAITING_TIME;
}
}

#endif  // TD_FILTERS_H
