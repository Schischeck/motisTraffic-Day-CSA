#pragma once

namespace motis {
namespace ris {
struct ConnectionAssessmentMessage;
}  // namespace ris

namespace rt {
namespace handler {

struct context;

void handle_connection_assessment(
    motis::ris::ConnectionAssessmentMessage const*, context&);

}  // namespace handler
}  // namespace rt
}  // namespace motis
