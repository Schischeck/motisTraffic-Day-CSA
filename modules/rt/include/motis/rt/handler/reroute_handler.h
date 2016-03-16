#pragma once

namespace motis {
namespace ris {
struct RerouteMessage;
}  // namespace ris

namespace rt {
namespace handler {

struct context;

void handle_reroute(motis::ris::RerouteMessage const*, context&);

}  // namespace handler
}  // namespace rt
}  // namespace motis
