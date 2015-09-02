#pragma once

#include "motis/core/schedule/waiting_time_rules.h"

namespace motis {
namespace loader {

waiting_time_rules parse_waiting_time_rules(
    std::vector<std::string> const& category_names);

}  // loader
}  // motis
