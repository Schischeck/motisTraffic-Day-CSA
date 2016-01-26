#include "motis/connectionchecker/connectionchecker.h"

#include <iostream>
#include <fstream>
#include <string>

#include "boost/program_options.hpp"

#include "motis/core/journey/journey.h"
#include "motis/core/journey/journey_util.h"
#include "motis/core/journey/message_to_journeys.h"
#include "motis/core/journey/journeys_to_message.h"
#include "motis/core/schedule/edges.h"

#include "motis/connectionchecker/error.h"

using namespace flatbuffers;
using namespace motis::realtime;
using namespace motis::routing;
using namespace motis::module;
using boost::system::error_code;

namespace motis {
namespace connectionchecker {

boost::program_options::options_description connectionchecker::desc() {
  return {"ConnectionChecker Module"};
}

void connectionchecker::print(std::ostream&) const {}
void connectionchecker::init() {}

time scheduled_dep_time(hash_map<graph_event, delay_info*> const& graph_to_di,
                        station_node const* node, unsigned train_nr, time t) {
  int route_id = -1;
  for (auto const& rn : node->get_route_nodes()) {
    for (auto const& e : rn->_edges) {
      if (e.type() != edge::ROUTE_EDGE) {
        continue;
      }
      auto conn = e.get_connection(t);
      if (conn == nullptr || conn->_full_con->con_info->train_nr != train_nr) {
        continue;
      }

      route_id = rn->_route;
      goto route_id_found;
    }
  }
route_id_found:

  auto di = graph_to_di.find({node->_id, train_nr, true, t, route_id});
  if (di != end(graph_to_di)) {
    return di->second->_schedule_event._schedule_time;
  }
  return t;
}

time scheduled_arr_time(hash_map<graph_event, delay_info*> const& graph_to_di,
                        station_node const* node, unsigned train_nr, time t) {
  int route_id = -1;
  for (auto const& rn : node->get_route_nodes()) {
    for (auto const& e : rn->_incoming_edges) {
      if (e->type() != edge::ROUTE_EDGE) {
        continue;
      }
      auto conn = e->get_connection_reverse(t);
      if (conn == nullptr || conn->_full_con->con_info->train_nr != train_nr) {
        continue;
      }

      route_id = rn->_route;
      goto route_id_found;
    }
  }
route_id_found:

  auto di = graph_to_di.find({node->_id, train_nr, false, t, route_id});
  if (di != end(graph_to_di)) {
    return di->second->_schedule_event._schedule_time;
  }
  return t;
}

void connectionchecker::on_msg(msg_ptr msg, sid, callback cb) {
  auto journeys = message_to_journeys(msg->content<RoutingResponse const*>());

  auto sync = synced_sched<schedule_access::RO>();
  auto const& graph_to_delay_info = sync.sched().graph_to_delay_info;
  auto const& eva_to_station = sync.sched().eva_to_station;
  auto const& station_nodes = sync.sched().station_nodes;
  auto sched_begin = sync.sched().schedule_begin_;

  for (auto& journey : journeys) {
    foreach_light_connection(
        journey, [&](journey::transport const& transport,
                     journey::stop& tail_stop, journey::stop& head_stop) {

          {  // tail stop
            auto station = eva_to_station.find(tail_stop.eva_no);
            if (station == end(eva_to_station)) {
              return cb({}, error::failure);
            }

            auto const node = station_nodes[station->second->index].get();
            auto t = scheduled_dep_time(
                graph_to_delay_info, node, transport.train_nr,
                unix_to_motistime(sched_begin, tail_stop.departure.timestamp));

            tail_stop.departure.schedule_timestamp =
                motis_to_unixtime(sched_begin, t);
          }

          {  // head stop
            auto station = eva_to_station.find(head_stop.eva_no);
            if (station == end(eva_to_station)) {
              return cb({}, error::failure);
            }

            auto const node = station_nodes[station->second->index].get();
            auto t = scheduled_arr_time(
                graph_to_delay_info, node, transport.train_nr,
                unix_to_motistime(sched_begin, head_stop.arrival.timestamp));

            head_stop.arrival.schedule_timestamp =
                motis_to_unixtime(sched_begin, t);
          }
        });
  }

  return cb(journeys_to_message(journeys), error::ok);
}

}  // namespace connectionchecker
}  // namespace motis
