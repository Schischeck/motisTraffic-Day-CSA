#pragma once

namespace motis {
namespace ris {

struct DelayMessage;

}  // namespace ris

namespace realtime {

class realtime_schedule;

namespace handler {

void handle_delay(motis::ris::DelayMessage const* msg,
                  motis::realtime::realtime_schedule& ctx);

}  // namespace handler
}  // namespace realtime
}  // namespace motis
