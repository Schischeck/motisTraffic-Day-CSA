#pragma once

namespace motis {
namespace ris {
struct RerouteMessage;
}  // namespace ris

namespace rt {
namespace handler {

struct context;

void handle_reroute(context&, motis::ris::RerouteMessage const*);

}  // namespace handler
}  // namespace rt
}  // namespace motis
