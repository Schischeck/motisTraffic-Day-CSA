#pragma once

namespace motis {

time get_schedule_time(primary_trip_id const& id, time_t const graph_time,
                       event_type const ev_type) {}

time get_schedule_time(light_connection const& con) {}

}  // namespace motis
