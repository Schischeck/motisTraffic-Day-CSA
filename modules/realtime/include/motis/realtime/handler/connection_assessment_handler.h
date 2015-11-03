#pragma once

namespace motis {
namespace ris {

struct ConnectionAssessmentMessage;

}  // namespace ris

namespace realtime {

struct realtime_context;

namespace handler {

void handle_connection_assessment(
    motis::ris::ConnectionAssessmentMessage const* msg,
    motis::realtime::realtime_context& ctx);

}  // namespace handler
}  // namespace realtime
}  // namespace motis
