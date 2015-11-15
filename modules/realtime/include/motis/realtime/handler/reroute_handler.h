#pragma once

namespace motis {
namespace ris {

struct RerouteMessage;

}  // namespace ris

namespace realtime {

class realtime_schedule;

namespace handler {

void handle_reroute(motis::ris::RerouteMessage const* msg,
                    motis::realtime::realtime_schedule& ctx);

}  // namespace handler
}  // namespace realtime
}  // namespace motis
