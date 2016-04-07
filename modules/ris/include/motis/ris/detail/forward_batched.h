#pragma once

#include <ctime>

#include "motis/ris/database.h"

namespace motis {
namespace ris {
namespace detail {

void forward_batched(std::time_t const schedule_begin,
                     std::time_t const schedule_end,
                     std::time_t const start_time,  //
                     std::time_t const end_time,  //
                     db_ptr const& database);

}  // namespace detail
}  // namespace ris
}  // namespace motis
