#pragma once

namespace motis {
namespace ris {

struct DelayMessage;

}  // namespace ris

namespace realtime {

struct realtime_context;

namespace handler {

void handle_delay(motis::ris::DelayMessage const* msg,
                  motis::realtime::realtime_context& ctx);

}  // namespace handler
}  // namespace realtime
}  // namespace motis
