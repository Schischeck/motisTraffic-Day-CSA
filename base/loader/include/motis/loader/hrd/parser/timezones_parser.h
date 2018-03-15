#pragma once

#include "motis/loader/hrd/model/timezones.h"
#include "motis/loader/hrd/parse_config_inheritance.h"
#include "motis/loader/loaded_file.h"

namespace motis {
namespace loader {
namespace hrd {

timezones parse_timezones(loaded_file const&, loaded_file const&,
                          config const&);

}  // namespace hrd
}  // namespace loader
}  // namespace motis
