#pragma once

namespace motis {
namespace ris {

struct ConnectionDecisionMessage;

}  // namespace ris

namespace realtime {

struct realtime_context;

namespace handler {

void handle_connection_decision(
    motis::ris::ConnectionDecisionMessage const* msg,
    motis::realtime::realtime_context& ctx);

}  // namespace handler
}  // namespace realtime
}  // namespace motis
