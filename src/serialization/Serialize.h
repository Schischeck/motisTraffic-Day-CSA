#ifndef TD_PREPROCESS_SERIALIZE_H_
#define TD_PREPROCESS_SERIALIZE_H_

#include <string>

#include "serialization/Schedule.h"

namespace td {

int serialize(Schedule const& sched, std::string const& prefix);

}  // namespace td

#endif  // TD_PREPROCESS_SERIALIZE_H_