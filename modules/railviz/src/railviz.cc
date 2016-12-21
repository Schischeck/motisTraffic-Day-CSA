#include "motis/railviz/railviz.h"

#include "utl/concat.h"
#include "utl/to_vec.h"

#include "motis/core/access/bfs.h"
#include "motis/core/access/edge_access.h"
#include "motis/core/access/realtime_access.h"
#include "motis/core/access/service_access.h"
#include "motis/core/access/time_access.h"

#include "motis/core/conv/trip_conv.h"

#include "motis/module/context/get_schedule.h"

#include "motis/railviz/train_retriever.h"

using namespace motis::module;
using namespace flatbuffers;

namespace motis {
namespace railviz {

railviz::railviz() : module("RailViz", "railviz") {}

railviz::~railviz() = default;

void railviz::init(motis::module::registry& reg) {
  namespace p = std::placeholders;
  reg.register_op("/railviz/get_trains",
                  std::bind(&railviz::get_trains, this, p::_1));
  reg.register_op("/railviz/get_trips",
                  std::bind(&railviz::get_trips, this, p::_1));
  train_retriever_ =
      std::make_unique<train_retriever>(synced_sched<RO>().sched());
}

std::vector<Offset<Train>> events_to_trains(
    schedule const& sched, FlatBufferBuilder& fbb,
    std::vector<ev_key> const& evs,  //
    std::map<int, int>& routes, std::vector<Offset<Route>>& fbs_routes,
    std::vector<std::set<trip::route_edge>>& route_edges) {
  auto const get_route_segments = [&fbb, &sched](
      std::set<trip::route_edge> const& edges) {
    return fbb.CreateVector(utl::to_vec(edges, [&](trip::route_edge const& e) {
      auto const& from = *sched.stations_[e->from_->get_station()->id_];
      auto const& to = *sched.stations_[e->to_->get_station()->id_];
      return CreateSegment(
          fbb, fbb.CreateString(from.eva_nr_), fbb.CreateString(to.eva_nr_),
          CreatePolyline(
              fbb, fbb.CreateVector(std::vector<double>(
                       {from.width_, from.length_, to.width_, to.length_}))));
    }));
  };

  auto const get_trips = [&sched, &fbb](ev_key const& k) {
    return fbb.CreateVector(
        utl::to_vec(*sched.merged_trips_[k.lcon()->trips_],
                    [&](trip const* trp) { return to_fbs(sched, fbb, trp); }));
  };

  auto const get_route = [&fbb, &sched, &routes, &fbs_routes, &route_edges,
                          &get_route_segments](ev_key const& k) -> int {
    auto const insert = routes.emplace(k.get_node()->route_, fbs_routes.size());
    if (insert.second) {
      auto const edges = route_bfs(k, bfs_direction::BOTH, false);
      route_edges.emplace_back(edges);
      fbs_routes.emplace_back(CreateRoute(fbb, get_route_segments(edges)));
    }
    return insert.first->second;
  };

  auto const service_names = [&fbb, &sched](ev_key const& k) {
    std::vector<std::string> names;
    auto c_info = k.lcon()->full_con_->con_info_;
    while (c_info != nullptr) {
      names.push_back(get_service_name(sched, c_info));
      c_info = c_info->merged_with_;
    }
    return fbb.CreateVector(utl::to_vec(names, [&](std::string const& name) {
      return fbb.CreateString(name);
    }));
  };

  return utl::to_vec(evs, [&](ev_key const& dep) {
    auto const route = get_route(dep);
    auto const& edges = route_edges[route];
    auto const segment_idx =
        std::distance(begin(edges), edges.find(dep.route_edge_));
    auto const arr = dep.get_opposite();
    return CreateTrain(fbb, service_names(dep),
                       motis_to_unixtime(sched, dep.get_time()),
                       motis_to_unixtime(sched, arr.get_time()),
                       motis_to_unixtime(sched, get_schedule_time(sched, dep)),
                       motis_to_unixtime(sched, get_schedule_time(sched, arr)),
                       route, segment_idx, get_trips(dep));
  });
}

std::vector<Offset<Station>> get_stations(
    schedule const& sched, FlatBufferBuilder& fbb,
    std::vector<std::set<trip::route_edge>>& route_edges) {
  std::set<int> stations_indices;
  for (auto const& route : route_edges) {
    for (auto const& e : route) {
      stations_indices.emplace(e->from_->get_station()->id_);
      stations_indices.emplace(e->to_->get_station()->id_);
    }
  }

  return utl::to_vec(stations_indices, [&sched, &fbb](int const station_idx) {
    auto const& station = *sched.stations_[station_idx];
    auto const pos = Position(station.width_, station.length_);
    return CreateStation(fbb, fbb.CreateString(station.eva_nr_),
                         fbb.CreateString(station.name_), &pos);
  });
};

msg_ptr railviz::get_trains(msg_ptr const& msg) const {
  auto const req = motis_content(RailVizTrainsRequest, msg);
  auto const& sched = get_schedule();

  message_creator fbb;

  std::map<int, int> routes;
  std::vector<Offset<Route>> fbs_routes;
  std::vector<std::set<trip::route_edge>> route_edges;

  auto const fbs_trains = events_to_trains(
      sched, fbb,
      train_retriever_->trains(
          unix_to_motistime(sched, req->start_time()),
          unix_to_motistime(sched, req->end_time()), req->max_trains(),
          {{req->corner1()->lat(), req->corner1()->lng()},
           {req->corner2()->lat(), req->corner2()->lng()}}),
      routes, fbs_routes, route_edges);

  fbb.create_and_finish(
      MsgContent_RailVizTrainsResponse,
      CreateRailVizTrainsResponse(
          fbb, fbb.CreateVector(fbs_trains), fbb.CreateVector(fbs_routes),
          fbb.CreateVector(get_stations(sched, fbb, route_edges)))
          .Union());

  return make_msg(fbb);
}

msg_ptr railviz::get_trips(msg_ptr const& msg) const {
  auto const& sched = get_schedule();
  message_creator fbb;

  std::map<int, int> routes;
  std::vector<Offset<Route>> fbs_routes;
  std::vector<std::set<trip::route_edge>> route_edges;

  std::vector<Offset<Train>> fbs_trains;
  for (auto const& trip : *motis_content(RailVizTripsRequest, msg)->trips()) {
    auto const trp = from_fbs(sched, trip);
    auto const first_dep =
        ev_key{trp->edges_->at(0), trp->lcon_idx_, event_type::DEP};
    utl::concat(
        fbs_trains,
        events_to_trains(
            sched, fbb,
            utl::to_vec(route_bfs(first_dep, bfs_direction::BOTH, false),
                        [&trp](trip::route_edge const& e) {
                          return ev_key{e, trp->lcon_idx_, event_type::DEP};
                        }),
            routes, fbs_routes, route_edges));
  }

  fbb.create_and_finish(
      MsgContent_RailVizTrainsResponse,
      CreateRailVizTrainsResponse(
          fbb, fbb.CreateVector(fbs_trains), fbb.CreateVector(fbs_routes),
          fbb.CreateVector(get_stations(sched, fbb, route_edges)))
          .Union());

  return make_msg(fbb);
}

}  // namespace railviz
}  // namespace motis
