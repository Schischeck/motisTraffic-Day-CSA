#pragma once

namespace motis {

namespace ris {
struct AdditionMessage;
}  // namespace ris

namespace rt {
namespace handler {

struct context;

void handle_addition(motis::ris::AdditionMessage const*, context&);

}  // namespace handler
}  // namespace rt
}  // namespace motis
