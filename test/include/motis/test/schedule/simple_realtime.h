#pragma once

#include "motis/module/message.h"
#include "motis/loader/loader_options.h"

namespace motis {

struct schedule;

namespace test {
namespace schedule {
namespace simple_realtime {

static loader::loader_options dataset_opt("test/schedule/simple_realtime",
                                          "20151124", 2, false, true, false,
                                          true);

motis::module::msg_ptr get_ris_message();

}  // namespace simple_realtime
}  // namespace schedule
}  // namespace test
}  // namespace motis
