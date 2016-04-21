#pragma once

namespace motis {
namespace ris {
struct AdditionMessage;
}  // namespace ris

namespace rt {
namespace handler {

struct context;

void handle_addition(context&, motis::ris::AdditionMessage const*);

}  // namespace handler
}  // namespace rt
}  // namespace motis
