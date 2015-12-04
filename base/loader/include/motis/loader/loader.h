#pragma once

#include "motis/core/schedule/schedule.h"

#include "motis/loader/loader_options.h"

namespace motis {
namespace loader {

schedule_ptr load_schedule(loader_options const&);

}  // namespace loader
}  // namespace motis
