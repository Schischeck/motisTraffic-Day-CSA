#include "motis/lookup/lookup.h"

#include <iostream>
#include <numeric>

#include "boost/program_options.hpp"

#include "motis/core/common/util.h"
#include "motis/core/journey/journey.h"
#include "motis/core/journey/journeys_to_message.h"
#include "motis/core/schedule/edge_access.h"
#include "motis/loader/util.h"
#include "motis/lookup/error.h"
#include "motis/lookup/station_geo_index.h"
#include "motis/protocol/Message_generated.h"

using namespace flatbuffers;
using namespace motis::module;
namespace po = boost::program_options;
using motis::loader::transform_to_vec;

namespace motis {
namespace lookup {

struct coordinate {
  double lat, lng;
};

template <typename T>
constexpr T identity(T&& v) {
  return std::forward<T>(v);
}

struct lookup::impl {
  explicit impl(std::vector<station_ptr> const& stations)
      : station_index_(stations) {}

  template <typename F>
  std::vector<typename std::result_of<F(station const*)>::type> close_stations(
      double const lat, double const lng, double const radius,
      F func = identity) const {
    auto stations = station_index_.stations(lat, lng, radius);
    return transform_to_vec(begin(stations), end(stations), func);
  }

  station_geo_index station_index_;
};

lookup::lookup() = default;
lookup::~lookup() = default;

po::options_description lookup::desc() {
  po::options_description desc("lookup Module");
  return desc;
}

void lookup::print(std::ostream&) const {}

void lookup::init() {
  impl_ = make_unique<impl>(synced_sched<RO>().sched().stations);
}

void lookup::on_msg(msg_ptr msg, sid, callback cb) {
  try {
    auto type = msg->content_type();
    switch (type) {
      case MsgContent_LookupGeoIndexRequest: {
        auto req = msg->content<LookupGeoIndexRequest const*>();
        return lookup_station(req, cb);
      }
      case MsgContent_LookupStationEventsRequest: {
        auto req = msg->content<LookupStationEventsRequest const*>();
        return lookup_station_events(req, cb);
      }
      case MsgContent_LookupTrainRequest: {
        auto req = msg->content<LookupTrainRequest const*>();
        return lookup_train(req, cb);
      }
      default: return cb({}, error::not_implemented);
    }
  } catch (boost::system::system_error const& e) {
    return cb({}, e.code());
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
    for (auto const& edge : route_node->_incoming_edges) {
      foreach_arrival_in(*edge, begin, end, [&](light_connection const& lcon) {
        auto const& info = *lcon._full_con->con_info;
        auto a_time = motis_to_unixtime(sched_begin, lcon.a_time);

        events.push_back(
            CreateEvent(b, EventType_Arrival, info.train_nr, a_time));
      });
    }

    for (auto const& edge : route_node->_edges) {
      foreach_departure_in(edge, begin, end, [&](light_connection const& lcon) {
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
                                              uint32_t train_nr, time t,
                                              bool is_departure) {
  if (is_departure) {
    for (auto const& rn : node->get_route_nodes()) {
      for (auto const& e : rn->_edges) {
        if (e.type() != edge::ROUTE_EDGE) {
          continue;
        }
        auto c = e.get_connection(t);
        if (c == nullptr || c->_full_con->con_info->train_nr != train_nr) {
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
        if (c == nullptr || c->_full_con->con_info->train_nr != train_nr) {
          continue;
        }
        return {rn->_route, std::distance(begin(e->_m._route_edge._conns), c)};
      }
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
      get_route_id_and_position(station_node, req->train_nr(), t,
                                req->event_type() == EventType_Departure);

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
    dep.platform = std::to_string(lcon._full_con->d_platform);

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
    arr.platform = std::to_string(lcon._full_con->d_platform);
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

void lookup::lookup_station(LookupGeoIndexRequest const* req, callback cb) {
  MessageCreator fbb;
  auto const storeStation = [&fbb](station const* s) {
    return CreateStation(fbb, fbb.CreateString(s->name),
                         fbb.CreateString(s->eva_nr), s->width, s->length);
  };

  std::vector<Offset<StationList>> list;
  for (auto const& c : *req->coordinates()) {
    list.push_back(CreateStationList(
        fbb, fbb.CreateVector(impl_->close_stations(
                 c->lat(), c->lng(), c->radius(), storeStation))));
  }

  fbb.CreateAndFinish(
      MsgContent_LookupGeoIndexResponse,
      CreateLookupGeoIndexResponse(fbb, fbb.CreateVector(list)).Union());

  return cb(make_msg(fbb), error::ok);
}

}  // namespace lookup
}  // namespace motis
