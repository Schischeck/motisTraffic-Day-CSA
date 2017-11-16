#pragma once

#include <ctime>

#include "motis/ris/database.h"

namespace motis {
namespace ris {
namespace detail {

constexpr auto kForwardBatchedInterval = 3600l;

std::time_t forward_batched(std::time_t sched_begin, std::time_t sched_end,
                            std::time_t end_time, db_ptr const& db);

std::time_t forward_batched(std::time_t sched_begin, std::time_t sched_end,
                            std::time_t start_time, std::time_t end_time,
                            db_ptr const& db);

}  // namespace detail
}  // namespace ris
}  // namespace motis
