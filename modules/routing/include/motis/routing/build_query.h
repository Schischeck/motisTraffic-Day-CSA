#pragma once

#include <algorithm>
#include <string>

#include "motis/core/schedule/schedule.h"
#include "motis/core/access/time_access.h"
#include "motis/core/access/trip_access.h"
#include "motis/core/access/trip_iterator.h"
#include "motis/core/conv/trip_conv.h"
#include "motis/module/context/motis_call.h"

#include "motis/routing/error.h"
#include "motis/routing/search.h"

#include "motis/protocol/RoutingRequest_generated.h"

namespace motis {
namespace routing {

inline void verify_timestamp(schedule const& sched, time_t t) {
  if (t < external_schedule_begin(sched) || t >= external_schedule_end(sched)) {
    throw std::system_error(error::journey_date_not_in_schedule);
  }
}

inline station_node const* get_station_node(schedule const& sched,
                                            InputStation const* input_station) {
  using guesser::StationGuesserResponse;

  std::string station_id;

  if (input_station->id()->Length() != 0) {
    station_id = input_station->id()->str();
  } else {
    module::message_creator b;
    b.create_and_finish(MsgContent_StationGuesserRequest,
                        guesser::CreateStationGuesserRequest(
                            b, 1, b.CreateString(input_station->name()->str()))
                            .Union(),
                        "/guesser");
    auto const msg = motis_call(make_msg(b))->val();
    auto const guesses = motis_content(StationGuesserResponse, msg)->guesses();
    if (guesses->size() == 0) {
      throw std::system_error(error::no_guess_for_station);
    }
    station_id = guesses->Get(0)->id()->str();
  }

  return motis::get_station_node(sched, station_id);
}

inline node const* get_route_node(schedule const& sched, TripId const* trip,
                                  station_node const* station,
                                  time arrival_time) {
  auto const stops = access::stops(from_fbs(sched, trip));
  auto const stop_it = std::find_if(
      begin(stops), end(stops), [&](access::trip_stop const& stop) {
        return stop.get_route_node()->station_node_ == station &&
               stop.arr_lcon().a_time_ == arrival_time;
      });
  if (stop_it == end(stops)) {
    throw std::system_error(error::event_not_found);
  }
  return (*stop_it).get_route_node();
}

inline search_query build_query(schedule const& sched,
                                RoutingRequest const* req) {
  search_query q;

  switch (req->start_type()) {
    case Start_PretripStart: {
      auto const start = reinterpret_cast<PretripStart const*>(req->start());
      verify_timestamp(sched, start->interval()->begin());
      verify_timestamp(sched, start->interval()->end());

      q.from_ = get_station_node(sched, start->station());
      q.interval_begin_ = unix_to_motistime(sched, start->interval()->begin());
      q.interval_end_ = unix_to_motistime(sched, start->interval()->end());
      break;
    }

    case Start_OntripStationStart: {
      auto start = reinterpret_cast<OntripStationStart const*>(req->start());
      verify_timestamp(sched, start->departure_time());

      q.from_ = get_station_node(sched, start->station());
      q.interval_begin_ = unix_to_motistime(sched, start->departure_time());
      q.interval_end_ = INVALID_TIME;
      break;
    }

    case Start_OntripTrainStart: {
      auto start = reinterpret_cast<OntripTrainStart const*>(req->start());
      q.from_ = get_route_node(sched, start->trip(),
                               get_station_node(sched, start->station()),
                               unix_to_motistime(sched, start->arrival_time()));
      q.interval_begin_ = unix_to_motistime(sched, start->arrival_time());
      q.interval_end_ = INVALID_TIME;
      break;
    }

    case Start_NONE: assert(false);
  }

  q.sched_ = &sched;
  q.to_ = get_station_node(sched, req->destination());
  q.query_edges_ = create_additional_edges(req->additional_edges(), sched);

  // TODO(Felix Guendling) remove when more edge types are supported
  if (req->search_dir() == SearchDir_Backward &&
      std::any_of(begin(q.query_edges_), end(q.query_edges_),
                  [](edge const& e) { return e.type() != edge::MUMO_EDGE; })) {
    throw std::system_error(error::edge_type_not_supported);
  }

  return q;
}

}  // namespace routing
}  // namespace motis
