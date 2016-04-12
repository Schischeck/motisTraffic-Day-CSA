#include "motis/ris/detail/forward_batched.h"

#include <algorithm>

#include "ctx/future.h"

#include "motis/core/common/logging.h"
#include "motis/module/context/motis_publish.h"
#include "motis/ris/detail/max_timestamp.h"
#include "motis/ris/detail/pack_msgs.h"
#include "motis/ris/ris_message.h"

using namespace motis::logging;
using namespace motis::module;

namespace motis {
namespace ris {
namespace detail {

std::string to_string(std::time_t const time) {
  char buf[sizeof "2011-10-08t07:07:09z-0430"];
  strftime(buf, sizeof buf, "%FT%TZ%z", gmtime(&time));
  return buf;
}

template <typename I, typename Fn>
void batched(I start, I end, I interval, Fn fn) {
  I curr = start;
  I next = start;

  while (true) {
    curr = next;
    next = std::min(curr + interval, end);
    if (next == curr) {
      return;
    }

    fn(curr, next);
  }
}

std::time_t forward_batched(std::time_t const sched_begin,
                            std::time_t const sched_end,
                            std::time_t const end_time, db_ptr const& db) {
  auto start_time = db_get_forward_start_time(db, sched_begin, sched_end);
  if (start_time == kDBInvalidTimestamp) {
    return 0l;
  }

  // first timestamp is excluded in db query
  return forward_batched(sched_begin, sched_end, start_time, end_time, db);
}

std::time_t forward_batched(std::time_t const sched_begin,
                            std::time_t const sched_end,
                            std::time_t const start_time,
                            std::time_t const end_time, db_ptr const& db) {
  std::time_t timestamp = 0;
  std::vector<future> futures;
  batched<std::time_t>(
      start_time, end_time, kForwardBatchedInterval,
      [&](std::time_t curr, std::time_t next) {
        manual_timer t{"database io"};
        auto msgs = db_get_messages(db, sched_begin, sched_end, curr, next);
        t.stop_and_print();

        if (!msgs.empty()) {
          timestamp = std::max(timestamp, max_timestamp(msgs));
          ctx::await_all(futures);
          LOG(info) << "RIS forwarding time to " << to_string(next);
          futures = motis_publish(pack_msgs(msgs));
        }
      });

  ctx::await_all(futures);
  LOG(info) << "RIS forwarded time to " << to_string(end_time);

  return timestamp;
}

}  // namespace detail
}  // namespace ris
}  // namespace motis
