#pragma once

namespace motis {
namespace ris {
struct DelayMessage;
}  // namespace ris

namespace rt {
namespace handler {

struct context;

void handle_delay(context&, motis::ris::DelayMessage const*);

}  // namespace handler
}  // namespace rt
}  // namespace motis
