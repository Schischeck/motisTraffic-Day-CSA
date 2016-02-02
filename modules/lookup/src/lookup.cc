#include "motis/lookup/lookup.h"

#include <iostream>
#include <numeric>

#include "boost/program_options.hpp"

#include "motis/core/journey/journey.h"
#include "motis/core/journey/journeys_to_message.h"
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
  } else if (type == MsgContent_LookupTrainRequest) {
    auto req = msg->content<LookupTrainRequest const*>();
    return lookup_train(req, cb);
  }

  return cb({}, error::not_implemented);
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

  // TODO only one binary search
  for (auto it = lb; it != ub; std::advance(it, 1)) {
    fun(*it);
  }
}

station_node* find_station_node(schedule const& sched,
                                std::string const& eva_nr) {
  auto it = sched.eva_to_station.find(eva_nr);
  if (it == end(sched.eva_to_station)) {
    throw boost::system::system_error(error::station_not_found);
  }
  return sched.station_nodes[it->second->index].get();
}

void lookup::lookup_station_events(LookupStationEventsRequest const* req,
                                   callback cb) {
  auto lock = synced_sched<schedule_access::RO>();
  auto const& schedule = lock.sched();
  auto sched_begin = lock.sched().schedule_begin_;

  auto station_node = find_station_node(schedule, req->eva_nr()->str());
  auto begin = unix_to_motistime(sched_begin, req->begin());
  auto end = unix_to_motistime(sched_begin, req->end());

  MessageCreator b;
  std::vector<Offset<Event>> events;
  for (auto const& route_node : station_node->get_route_nodes()) {
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
  };

  b.CreateAndFinish(
      MsgContent_LookupStationEventsResponse,
      CreateLookupStationEventsResponse(b, b.CreateVector(events)).Union());

  return cb(make_msg(b), error::ok);
}

std::pair<int, int> get_route_id_and_position(station_node const* node,
                                              unsigned train_nr, time t,
                                              bool is_departure) {
  if (!is_departure) {
    throw boost::system::system_error(error::not_implemented);
  }

  for (auto const& rn : node->get_route_nodes()) {
    for (auto const& e : rn->_edges) {
      if (e.type() != edge::ROUTE_EDGE) {
        continue;
      }
      auto conn = e.get_connection(t);
      if (conn == nullptr || conn->_full_con->con_info->train_nr != train_nr) {
        continue;
      }

      return {rn->_route, std::distance(begin(e._m._route_edge._conns), conn)};
    }
  }

  throw boost::system::system_error(error::route_not_found);
}

// simple case -> each route node has one route edge (no merge split)
edge* get_outgoing_route_edge(node* node) {
  for (auto& edge : node->_edges) {
    if (edge.type() == edge::ROUTE_EDGE) {
      return &edge;
    }
  }
  return nullptr;
}

void lookup::lookup_train(LookupTrainRequest const* req, callback cb) {
  auto lock = synced_sched<schedule_access::RO>();
  auto const& schedule = lock.sched();
  auto sched_begin = lock.sched().schedule_begin_;

  auto station_node = find_station_node(schedule, req->eva_nr()->str());
  auto t = unix_to_motistime(sched_begin, req->time());

  auto route =
      get_route_id_and_position(station_node, req->train_nr(), t, true);

  int i = 0;
  journey j;

  auto* route_node = schedule.route_index_to_first_route_node[route.first];
  auto* route_edge = get_outgoing_route_edge(route_node);
  while (route_edge != nullptr) {
    auto const& lcon = route_edge->_m._route_edge._conns[route.second];
    auto const& station = schedule.stations[route_node->get_station()->_id];

    if (j.stops.empty()) {
      journey::stop d_stop;
      d_stop.index = i++;
      d_stop.interchange = false;
      d_stop.eva_no = station->eva_nr;
      d_stop.name = station->name;
      d_stop.lat = station->lat();
      d_stop.lng = station->lng();

      j.stops.push_back(d_stop);
    }

    std::time_t d_time = motis_to_unixtime(sched_begin, lcon.d_time);

    journey::stop::event_info dep;
    dep.valid = true;
    dep.timestamp = d_time;
    dep.schedule_timestamp = d_time;
    dep.platform = lcon._full_con->d_platform;

    j.stops.back().departure = dep;

    auto const& next_station =
        schedule.stations[route_edge->get_destination()->get_station()->_id];

    journey::stop a_stop;
    a_stop.index = i++;
    a_stop.interchange = false;
    a_stop.eva_no = next_station->eva_nr;
    a_stop.name = next_station->name;
    a_stop.lat = next_station->lat();
    a_stop.lng = next_station->lng();

    std::time_t a_time = motis_to_unixtime(sched_begin, lcon.a_time);

    journey::stop::event_info arr;
    arr.valid = true;
    arr.timestamp = a_time;
    arr.schedule_timestamp = a_time;
    arr.platform = lcon._full_con->d_platform;
    a_stop.arrival = arr;

    j.stops.push_back(a_stop);

    route_node = route_edge->get_destination();
    route_edge = get_outgoing_route_edge(route_node);
  }

  MessageCreator b;
  b.CreateAndFinish(MsgContent_LookupTrainResponse,
                    CreateLookupTrainResponse(b, to_connection(b, j)).Union());
  cb(make_msg(b), error::ok);
}

}  // namespace lookup
}  // namespace motis
