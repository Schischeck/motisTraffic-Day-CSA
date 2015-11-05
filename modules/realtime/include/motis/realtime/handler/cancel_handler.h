#pragma once

namespace motis {
namespace ris {

struct CancelMessage;

}  // namespace ris

namespace realtime {

struct realtime_context;

namespace handler {

void handle_cancel(motis::ris::CancelMessage const* msg,
                   motis::realtime::realtime_context& ctx);

}  // namespace handler
}  // namespace realtime
}  // namespace motis
