#pragma once

#include "motis/module/message.h"
#include "motis/loader/loader_options.h"

namespace motis {
namespace test {
namespace schedule {
namespace simple_realtime {

static loader::loader_options dataset_opt("test/schedule/simple_realtime",
                                          "20151124");

motis::module::msg_ptr get_ris_message();

}  // namespace simple_realtime
}  // namespace schedule
}  // namespace test
}  // namespace motis
