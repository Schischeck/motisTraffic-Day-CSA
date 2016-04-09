#pragma once

namespace motis {
namespace ris {
struct CancelMessage;
}  // namespace ris

namespace rt {
namespace handler {

struct context;

void handle_cancel(context&, motis::ris::CancelMessage const*);

}  // namespace handler
}  // namespace rt
}  // namespace motis
