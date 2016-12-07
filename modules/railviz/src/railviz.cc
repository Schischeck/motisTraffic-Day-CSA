#include "motis/railviz/railviz.h"

#include "motis/core/access/bfs.h"
#include "motis/core/access/edge_access.h"
#include "motis/core/access/realtime_access.h"
#include "motis/core/access/time_access.h"

#include "motis/core/conv/trip_conv.h"

#include "motis/module/context/get_schedule.h"

#include "motis/railviz/train_retriever.h"

using namespace motis::module;
using namespace flatbuffers;

constexpr auto MAX_TRAINS = 256;

namespace motis {
namespace railviz {

railviz::railviz() : module("RailViz", "railviz") {}

railviz::~railviz() = default;

void railviz::init(motis::module::registry& reg) {
  namespace p = std::placeholders;
  reg.register_op("/railviz/get_trains",
                  std::bind(&railviz::get_trains, this, p::_1));
  train_retriever_ =
      std::make_unique<train_retriever>(synced_sched<RO>().sched());
}

msg_ptr railviz::get_trains(msg_ptr const& msg) const {
  auto const con = motis_content(RailVizTrainsRequest, msg);
  auto const& sched = get_schedule();

  message_creator fbb;

  auto const get_route_segments =
      [&fbb, &sched](std::set<trip::route_edge> const& edges) {
        return fbb.CreateVector(
            transform_to_vec(edges, [&](trip::route_edge const& e) {
              auto const from_id = e->from_->get_station()->id_;
              auto const to_id = e->to_->get_station()->id_;
              return CreateSegment(
                  fbb, from_id, to_id,
                  CreatePolyline(fbb, fbb.CreateVector(std::vector<double>(
                                          {sched.stations_[from_id]->width_,
                                           sched.stations_[from_id]->length_,
                                           sched.stations_[to_id]->width_,
                                           sched.stations_[to_id]->length_}))));
            }));
      };

  auto const get_trips = [&sched, &fbb](ev_key const& k) {
    return fbb.CreateVector(transform_to_vec(
        *sched.merged_trips_[k.lcon()->trips_],
        [&](trip const* trp) { return to_fbs(sched, fbb, trp); }));
  };

  std::map<int, int> routes;
  std::vector<Offset<Route>> fbs_routes;
  std::vector<std::set<trip::route_edge>> route_edges;
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

  auto const get_stations = [&sched, &route_edges]() {
    std::set<int> stations_indices;
    for (auto const& route : route_edges) {
      for (auto const& e : route) {
        stations_indices.emplace(e->from_->get_station()->id_);
        stations_indices.emplace(e->to_->get_station()->id_);
      }
    }

    return transform_to_vec_no_default_ctor(
        stations_indices, [&sched](int const station_idx) {
          auto const& station = *sched.stations_[station_idx];
          return Position(station.width_, station.length_);
        });
  };

  auto const fbs_trains = fbb.CreateVector(transform_to_vec(
      train_retriever_->trains(
          unix_to_motistime(sched, con->start_time()),
          unix_to_motistime(sched, con->end_time()), MAX_TRAINS,
          std::make_pair(
              geo::coord{con->corner1()->lat(), con->corner1()->lng()},
              geo::coord{con->corner2()->lat(), con->corner2()->lng()})),
      [&](ev_key const& dep) {
        auto const route = get_route(dep);
        auto const& edges = route_edges[route];
        auto const segment_idx =
            std::distance(begin(edges), edges.find(dep.route_edge_));
        auto const arr = dep.get_opposite();
        return CreateTrain(
            fbb, motis_to_unixtime(sched, dep.get_time()),
            motis_to_unixtime(sched, arr.get_time()),
            motis_to_unixtime(sched, get_schedule_time(sched, dep)),
            motis_to_unixtime(sched, get_schedule_time(sched, arr)), route,
            segment_idx, get_trips(dep));
      }));

  fbb.create_and_finish(
      MsgContent_RailVizTrainsResponse,
      CreateRailVizTrainsResponse(fbb, fbs_trains, fbb.CreateVector(fbs_routes),
                                  fbb.CreateVectorOfStructs(get_stations()))
          .Union());

  return make_msg(fbb);
}

}  // namespace railviz
}  // namespace motis
