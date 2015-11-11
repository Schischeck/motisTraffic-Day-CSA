#pragma once

namespace motis {
namespace ris {

struct ConnectionAssessmentMessage;

}  // namespace ris

namespace realtime {

class realtime_schedule;

namespace handler {

void handle_connection_assessment(
    motis::ris::ConnectionAssessmentMessage const* msg,
    motis::realtime::realtime_schedule& ctx);

}  // namespace handler
}  // namespace realtime
}  // namespace motis
