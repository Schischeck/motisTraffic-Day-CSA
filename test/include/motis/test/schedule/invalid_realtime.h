#pragma once

#include "motis/module/message.h"
#include "motis/loader/loader_options.h"

namespace motis {

struct schedule;

namespace test {
namespace schedule {
namespace invalid_realtime {

static loader::loader_options dataset_opt("test/schedule/invalid_realtime",
                                          "20151124", 2, false, true, false,
                                          true);

motis::module::msg_ptr get_trip_conflict_ris_message(motis::schedule const&);
motis::module::msg_ptr get_ts_conflict_ris_message(motis::schedule const&);
motis::module::msg_ptr get_additional_ris_message(motis::schedule const&);

}  // namespace invalid_realtime
}  // namespace schedule
}  // namespace test
}  // namespace motis
