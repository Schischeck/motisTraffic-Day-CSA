#pragma once

namespace motis {
namespace ris {
struct ConnectionDecisionMessage;
}  // namespace ris

namespace rt {
namespace handler {

struct context;

void handle_connection_decision(context&,
                                motis::ris::ConnectionDecisionMessage const*);

}  // namespace handler
}  // namespace rt
}  // namespace motis
