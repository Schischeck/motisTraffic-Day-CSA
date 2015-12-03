#pragma once

#include "motis/loader/loaded_file.h"
#include "motis/loader/hrd/model/timezones.h"

namespace motis {
namespace loader {
namespace hrd {

timezones parse_timezones(loaded_file const&, loaded_file const&);

}  // hrd
}  // loader
}  // motis
