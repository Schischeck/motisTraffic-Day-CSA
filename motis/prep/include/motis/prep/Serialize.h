#ifndef TD_PREPROCESS_SERIALIZE_H_
#define TD_PREPROCESS_SERIALIZE_H_

#include <string>

#include "motis/core/schedule/schedule.h"

namespace td {

int serialize(text_schedule const& sched, std::string const& prefix);

}  // namespace td

#endif  // TD_PREPROCESS_SERIALIZE_H_
