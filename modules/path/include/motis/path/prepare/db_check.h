#pragma once

#include "motis/path/db/kv_database.h"

namespace motis {
namespace path {

void check_databases(kv_database const& expected, kv_database const& actual);

} // namespace path
} // namespace motis
