#ifndef TD_LOADER_H_
#define TD_LOADER_H_

#include "motis/core/schedule/Schedule.h"

namespace td {

SchedulePtr loadSchedule(std::string const& prefix);
SchedulePtr loadTextSchedule(std::string const& prefix);
SchedulePtr loadBinarySchedule(std::string const& prefix);

}  // namespace td

#endif  // TD_LOADER_H_