#pragma once

namespace motis {
struct trip;

namespace ris {
struct TripId;
}  // namespace ris

namespace rt {
namespace handler {

trip const* get_trip(schedule const&, motis::ris::TripId const*,
                     message_counter&);

}  // namespace handler
}  // namespace rt
}  // namespace motis
