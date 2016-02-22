#include "motis/lookup/lookup_id_train.h"

#include "motis/core/journey/journey.h"
#include "motis/core/journey/journeys_to_message.h"
#include "motis/lookup/util.h"

using namespace flatbuffers;
using namespace motis::routing;

namespace motis {
namespace lookup {

// TODO ideally, an index id_event-> first_route_node+position would be used
//      however: this index needs to be maintained by the realtime module

std::pair<int, int> get_route_id_and_position(station_node const* node,
                                              bool is_departure,
                                              uint32_t const train_nr,
                                              std::string const& line_id,
                                              time const t) {
  if (is_departure) {
    for (auto const& rn : node->get_route_nodes()) {
      for (auto const& e : rn->_edges) {
        if (e.type() != edge::ROUTE_EDGE) {
          continue;
        }
        auto c = e.get_connection(t);
        if (c == nullptr) {
          continue;
        }
        auto info = c->_full_con->con_info;
        if (info->train_nr != train_nr || info->line_identifier != line_id) {
          continue;
        }
        return {rn->_route, std::distance(begin(e._m._route_edge._conns), c)};
      }
    }
  } else {
    for (auto const& rn : node->get_route_nodes()) {
      for (auto const& e : rn->_incoming_edges) {
        if (e->type() != edge::ROUTE_EDGE) {
          continue;
        }
        auto c = e->get_connection_reverse(t);
        if (c == nullptr) {
          continue;
        }
        auto info = c->_full_con->con_info;
        if (info->train_nr != train_nr || info->line_identifier != line_id) {
          continue;
        }
        return {rn->_route, std::distance(begin(e->_m._route_edge._conns), c)};
      }
    }
  }

  throw boost::system::system_error(error::route_not_found);
}

Offset<Connection> lookup_id_train(FlatBufferBuilder& fbb,
                                   schedule const& sched,
                                   IdEvent const* id_event) {
  auto const& station = get_station_node(sched, id_event->eva_nr()->str());
  auto const& is_dep = id_event->type() == EventType_Departure;
  auto const& train_nr = id_event->train_nr();
  auto const& line_id = id_event->line_id()->str();
  auto const& t = unix_to_motistime(sched, id_event->schedule_time());

  auto route = get_route_id_and_position(station, is_dep, train_nr, line_id, t);

  int i = 0;
  journey j;

  auto* route_node = sched.route_index_to_first_route_node[route.first];
  auto* route_edge = find_outgoing_route_edge(route_node);
  while (route_edge != nullptr) {
    auto const& lcon = route_edge->_m._route_edge._conns[route.second];
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

    auto schedule_d_time = get_schedule_time(sched, station_id, train_nr, true,
                                             lcon.d_time, route_node->_route);

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
        get_schedule_time(sched, next_station_id, train_nr, false, lcon.a_time,
                          route_node->_route);

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
