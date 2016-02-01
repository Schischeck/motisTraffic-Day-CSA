#include "motis/lookup/lookup.h"

#include <iostream>
#include <numeric>

#include "boost/program_options.hpp"

#include "motis/lookup/error.h"
#include "motis/protocol/Message_generated.h"

using namespace flatbuffers;
using namespace motis::module;
namespace po = boost::program_options;

namespace motis {
namespace lookup {

po::options_description lookup::desc() {
  po::options_description desc("lookup Module");
  return desc;
}

void lookup::print(std::ostream&) const {}

void lookup::init() {}

void lookup::on_msg(msg_ptr msg, sid, callback cb) {
  auto type = msg->content_type();
  if (type == MsgContent_LookupStationEventsRequest) {
    auto req = msg->content<LookupStationEventsRequest const*>();
    return lookup_station_events(req, cb);
  }

  return cb({}, error::not_implemented);
}

template <typename F>
void foreach_route_node(node const& station_node, F fun) {
  for (auto const& edge : station_node._edges) {
    auto const* route_node = edge.get_destination();
    if (!route_node->is_route_node()) {
      continue;
    }

    fun(route_node);
  }
}

template <typename F>
void foreach_departure_event_in(edge const& edge, time begin, time end, F fun) {
  if (edge.type() != edge::ROUTE_EDGE) {
    return;
  }

  auto const& conns = edge._m._route_edge._conns;
  if (conns.size() == 0) {
    return;
  }

  auto lb = std::lower_bound(std::begin(conns), std::end(conns),
                             light_connection(begin));
  auto ub = std::upper_bound(std::begin(conns), std::end(conns),
                             light_connection(end));

  for (auto it = lb; it != ub; std::advance(it, 1)) {
    fun(*it);
  }
}

void lookup::lookup_station_events(LookupStationEventsRequest const* req,
                                   callback cb) {
  auto lock = synced_sched<schedule_access::RO>();
  auto const& schedule = lock.sched();
  auto sched_begin = lock.sched().schedule_begin_;

  auto it = schedule.eva_to_station.find(req->eva_nr()->str());
  if (it == end(schedule.eva_to_station)) {
    return cb({}, error::station_not_found);
  }
  auto const& station_node = schedule.station_nodes[it->second->index];

  auto begin = unix_to_motistime(sched_begin, req->begin());
  auto end = unix_to_motistime(sched_begin, req->end());

  MessageCreator b;
  std::vector<Offset<Event>> events;

  foreach_route_node(*station_node, [&](node const* route_node) {
    // TODO incomming edges

    for (auto const& edge : route_node->_edges) {
      foreach_departure_event_in(
          edge, begin, end, [&](light_connection const& lcon) {
            auto const& info = *lcon._full_con->con_info;
            auto d_time = motis_to_unixtime(sched_begin, lcon.d_time);

            events.push_back(
                CreateEvent(b, EventType_Departure, info.train_nr, d_time));
          });
    }
  });

  b.CreateAndFinish(
      MsgContent_LookupStationEventsResponse,
      CreateLookupStationEventsResponse(b, b.CreateVector(events)).Union());

  return cb(make_msg(b), error::ok);
};

}  // namespace lookup
}  // namespace motis
