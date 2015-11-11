#pragma once

namespace motis {
namespace ris {

struct AdditionMessage;

}  // namespace ris

namespace realtime {

class realtime_schedule;

namespace handler {

void handle_addition(motis::ris::AdditionMessage const* msg,
                     motis::realtime::realtime_schedule& ctx);

}  // namespace handler
}  // namespace realtime
}  // namespace motis
