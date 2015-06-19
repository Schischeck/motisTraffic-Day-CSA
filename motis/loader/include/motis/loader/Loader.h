#ifndef TD_LOADER_H_
#define TD_LOADER_H_

#include "motis/core/schedule/schedule.h"

namespace td {

schedule_ptr load_schedule(std::string const& prefix);
schedule_ptr load_text_schedule(std::string const& prefix);
schedule_ptr load_binary_schedule(std::string const& prefix);

}  // namespace td

#endif  // TD_LOADER_H_
