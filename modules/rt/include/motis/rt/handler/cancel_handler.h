#pragma once

namespace motis {
namespace ris {
struct CancelMessage;
}  // namespace ris

namespace rt {
namespace handler {

struct context;

void handle_cancel(motis::ris::CancelMessage const*, context&);

}  // namespace handler
}  // namespace rt
}  // namespace motis
