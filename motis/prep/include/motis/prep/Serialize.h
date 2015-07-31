#ifndef TD_PREPROCESS_SERIALIZE_H_
#define TD_PREPROCESS_SERIALIZE_H_

#include <string>

#include "motis/core/schedule/Schedule.h"

namespace td {

int serialize(TextSchedule const& sched, std::string const& prefix);

}  // namespace td

#endif  // TD_PREPROCESS_SERIALIZE_H_