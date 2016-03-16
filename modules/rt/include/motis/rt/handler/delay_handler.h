#pragma once

namespace motis {
namespace ris {
struct DelayMessage;
}  // namespace ris

namespace rt {
namespace handler {

struct context;

void handle_delay(motis::ris::DelayMessage const*, context&);

}  // namespace handler
}  // namespace rt
}  // namespace motis
