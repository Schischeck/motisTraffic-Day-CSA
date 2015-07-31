#ifndef TD_FILTERS_H
#define TD_FILTERS_H TD_FILTERS_H

#include <algorithm>

#include "motis/core/schedule/Connection.h"

#define MAX_TRAVEL_TIME  (1440)
#define MAX_TRANSFERS    (6)
#define MAX_WAITING_TIME (200)

namespace td
{

inline bool isFilteredTravelTime(unsigned const nTravelTime)
{ return nTravelTime > MAX_TRAVEL_TIME; }

inline bool isFilteredTransfers(unsigned const nTransfers)
{ return nTransfers > MAX_TRANSFERS; }

inline bool isFilteredWaitingTime(
    LightConnection const* con,
    unsigned predTravelTime,
    unsigned nextTravelTime)
{
  unsigned conTime = con != nullptr ? con->aTime - con->dTime : 0;
  return nextTravelTime - predTravelTime - conTime > MAX_WAITING_TIME;
}

}

#endif //TD_FILTERS_H
