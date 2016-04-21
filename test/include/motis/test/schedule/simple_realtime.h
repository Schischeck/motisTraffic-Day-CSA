#pragma once

#include "motis/module/message.h"

namespace motis {
namespace test {
namespace schedule {
namespace simple_realtime {

constexpr auto kSchedulePath = "test/schedule/simple_realtime";
constexpr auto kScheduleDate = "20151124";

motis::module::msg_ptr get_ris_message();

}  // namespace simple_realtime
}  // namespace schedule
}  // namespace test
}  // namespace motis
