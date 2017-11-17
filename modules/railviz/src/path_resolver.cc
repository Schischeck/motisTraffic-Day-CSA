#include "motis/railviz/path_resolver.h"

#include "utl/get_or_create.h"

#include "parser/util.h"

#include "motis/module/context/motis_call.h"
#include "motis/module/message.h"

using namespace motis::module;
using namespace flatbuffers;

namespace motis {
namespace railviz {

path_resolver::path_resolver(schedule const& sched, int zoom_level)
    : sched_(sched), zoom_level_(zoom_level), req_count_(0) {}

std::vector<std::vector<double>> path_resolver::get_trip_path(trip const* trp) {
  return utl::get_or_create(trip_cache_, trp->edges_, [&]() {
    ++req_count_;

    std::vector<std::string> ids;
    for (auto i = 0u; i < trp->edges_->size(); ++i) {
      if (i == 0) {
        ids.push_back(
            sched_.stations_.at(trp->edges_->at(i)->from_->get_station()->id_)
                ->eva_nr_);
      }
      ids.push_back(
          sched_.stations_.at(trp->edges_->at(i)->to_->get_station()->id_)
              ->eva_nr_);
    }

    message_creator fbb;
    fbb.create_and_finish(
        MsgContent_PathStationSeqRequest,
        path::CreatePathStationSeqRequest(
            fbb,
            fbb.CreateVector(utl::to_vec(
                ids,
                [&](std::string const& id) { return fbb.CreateString(id); })),
            trp->edges_->front()
                ->m_.route_edge_.conns_[trp->lcon_idx_]
                .full_con_->clasz_,
            zoom_level_, false)
            .Union(),
        "/path/station_seq");

    using path::PathSeqResponse;
    auto const path_res = motis_call(make_msg(fbb))->val();
    return utl::to_vec(
        *motis_content(PathSeqResponse, path_res)->segments(),
        [&](Polyline const* l) { return utl::to_vec(*l->coordinates()); });
  });
}

std::vector<double> path_resolver::get_segment_path(edge const* e) {
  verify(!e->empty(), "non-empty route edge needed");

  return utl::get_or_create(edge_cache_, e, [&]() {
    auto const trp =
        sched_.merged_trips_[e->m_.route_edge_.conns_.front().trips_]->front();

    try {
      auto const it =
          std::find_if(begin(*trp->edges_), end(*trp->edges_),
                       [&](edge const* trp_e) { return trp_e == e; });
      verify(it != end(*trp->edges_), "trip edge data error");

      auto segment =
          get_trip_path(trp).at(std::distance(begin(*trp->edges_), it));
      verify(segment.size() >= 4, "no empty segments allowed");

      return segment;
    } catch (std::exception const& ex) {
      auto const& from = *sched_.stations_[e->from_->get_station()->id_];
      auto const& to = *sched_.stations_[e->to_->get_station()->id_];
      return std::vector<double>(
          {from.width_, from.length_, to.width_, to.length_});
    }
  });
}

}  // namespace railviz
}  // namespace motis
