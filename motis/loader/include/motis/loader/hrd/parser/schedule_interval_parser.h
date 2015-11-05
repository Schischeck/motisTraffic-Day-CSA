#pragma once

#include "motis/schedule-format/Interval_generated.h"

#include "motis/loader/loaded_file.h"

namespace motis {
namespace loader {

Interval parse_interval(loaded_file const&);

}  // loader
}  // motis
