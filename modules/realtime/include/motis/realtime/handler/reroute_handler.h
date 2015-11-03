#pragma once

namespace motis {
namespace ris {

struct RerouteMessage;

}  // namespace ris

namespace realtime {

struct realtime_context;

namespace handler {

void handle_reroute(
    motis::ris::RerouteMessage const* msg,
    motis::realtime::realtime_context& ctx);

}  // namespace handler
}  // namespace realtime
}  // namespace motis
