#pragma once

#include <memory>

#include "motis/core/schedule/category.h"
#include "motis/core/schedule/waiting_time_rules.h"

namespace motis {
namespace loader {

waiting_time_rules load_waiting_time_rules(
    std::vector<std::unique_ptr<category>> const& category_ptrs);

}  // namespace loader
}  // namespace motis
