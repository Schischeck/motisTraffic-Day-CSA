#pragma once

#include "boost/date_time/gregorian/gregorian_types.hpp"

#include "motis/schedule-format/Interval_generated.h"

#include "motis/loader/loaded_file.h"

namespace motis {
namespace loader {
namespace hrd {

Interval parse_interval(loaded_file const&);

boost::gregorian::date get_first_schedule_date(loaded_file const& lf);

}  // namespace loader
}  // namespace motis
}  // namespace hrd
