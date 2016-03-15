#include "motis/lookup/lookup_id_train.h"

#include "motis/core/access/time_access.h"
#include "motis/core/access/trip_access.h"
#include "motis/core/journey/journey.h"
#include "motis/core/journey/journeys_to_message.h"
#include "motis/lookup/util.h"

using namespace flatbuffers;
using namespace motis::routing;

namespace motis {
namespace lookup {

Offset<Connection> lookup_id_train(FlatBufferBuilder& fbb,
                                   schedule const& sched, IdEvent const* e) {
  auto trp = get_trip(sched, e->eva_nr()->str(), e->train_nr(), e->time(),
                      e->target_eva_nr()->str(), e->target_time(),
                      e->type() == EventType_Arrival, e->line_id()->str());
  int i = 0;
  journey j;

  auto* route_edge = trp->edges->front();
  auto* route_node = route_edge->_from;
  while (route_edge != nullptr) {
    auto const& lcon = route_edge->_m._route_edge._conns[trp->lcon_idx];
    auto const& station_id = route_node->get_station()->_id;
    auto const& station = sched.stations[station_id];

    if (j.stops.empty()) {
      journey::stop d_stop;
      d_stop.index = i++;
      d_stop.interchange = false;
      d_stop.eva_no = station->eva_nr;
      d_stop.name = station->name;
      d_stop.lat = station->lat();
      d_stop.lng = station->lng();

      journey::stop::event_info arr;
      arr.valid = false;
      d_stop.arrival = arr;

      j.stops.push_back(d_stop);
    }

    auto schedule_d_time =
        get_schedule_time(sched, station_id, e->train_nr(), true, lcon.d_time,
                          route_node->_route);

    journey::stop::event_info dep;
    dep.valid = true;
    dep.timestamp = motis_to_unixtime(sched, lcon.d_time);
    dep.schedule_timestamp = motis_to_unixtime(sched, schedule_d_time);
    dep.platform = sched.tracks[lcon._full_con->d_platform];

    j.stops.back().departure = dep;

    auto next_station_id = route_edge->get_destination()->get_station()->_id;
    auto const& next_station = sched.stations[next_station_id];

    journey::stop a_stop;
    a_stop.index = i++;
    a_stop.interchange = false;
    a_stop.eva_no = next_station->eva_nr;
    a_stop.name = next_station->name;
    a_stop.lat = next_station->lat();
    a_stop.lng = next_station->lng();

    auto schedule_a_time =
        get_schedule_time(sched, next_station_id, e->train_nr(), false,
                          lcon.a_time, route_node->_route);

    journey::stop::event_info arr;
    arr.valid = true;
    arr.timestamp = motis_to_unixtime(sched, lcon.a_time);
    arr.schedule_timestamp = motis_to_unixtime(sched, schedule_a_time);
    arr.platform = sched.tracks[lcon._full_con->a_platform];
    a_stop.arrival = arr;

    j.stops.push_back(a_stop);

    route_node = route_edge->get_destination();
    route_edge = find_outgoing_route_edge(route_node);
  }

  journey::stop::event_info dep;
  dep.valid = false;
  j.stops.back().departure = dep;

  // TODO write transport
  return to_connection(fbb, j);
}

}  // namespace lookup
}  // namespace motis
