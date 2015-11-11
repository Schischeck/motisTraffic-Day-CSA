#pragma once

namespace motis {
namespace ris {

struct ConnectionDecisionMessage;

}  // namespace ris

namespace realtime {

class realtime_schedule;

namespace handler {

void handle_connection_decision(
    motis::ris::ConnectionDecisionMessage const* msg,
    motis::realtime::realtime_schedule& ctx);

}  // namespace handler
}  // namespace realtime
}  // namespace motis
