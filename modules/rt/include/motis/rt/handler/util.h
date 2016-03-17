#pragma once

namespace motis {
struct trip;

namespace ris {
struct TripId;
} // namespace ris

namespace rt {
namespace handler {

trip const* get_trip(context&, motis::ris::TripId const*);

}  // namespace handler
}  // namespace rt
}  // namespace motis
