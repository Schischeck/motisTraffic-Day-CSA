#pragma once

namespace motis {
namespace ris {

struct AdditionMessage;

}  // namespace ris

namespace realtime {

struct realtime_context;

namespace handler {

void handle_addition(motis::ris::AdditionMessage const* msg,
                     motis::realtime::realtime_context& ctx);

}  // namespace handler
}  // namespace realtime
}  // namespace motis
