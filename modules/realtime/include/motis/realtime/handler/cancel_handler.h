#pragma once

namespace motis {
namespace ris {

struct CancelMessage;

}  // namespace ris

namespace realtime {

class realtime_schedule;

namespace handler {

void handle_cancel(motis::ris::CancelMessage const* msg,
                   motis::realtime::realtime_schedule& ctx);

}  // namespace handler
}  // namespace realtime
}  // namespace motis
