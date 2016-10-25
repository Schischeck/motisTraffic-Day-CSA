#include "motis/cc/cc.h"

#include "parser/util.h"

#include "motis/core/access/realtime_access.h"
#include "motis/core/access/station_access.h"
#include "motis/core/conv/trip_conv.h"
#include "motis/core/journey/message_to_journeys.h"

#include "motis/module/context/get_schedule.h"

#include "boost/program_options.hpp"

namespace po = boost::program_options;
using namespace motis::module;

namespace motis {
namespace cc {

struct interchange {
  void reset() {
    enter_ = ev_key();
    leave_ = ev_key();
  }
  std::size_t leave_stop_idx_, enter_stop_idx_;
  ev_key leave_, enter_;
};

po::options_description cc::desc() {
  return po::options_description("RT Module");
}

void cc::init(motis::module::registry& reg) {
  namespace p = std::placeholders;
  reg.subscribe("/cc/check_journey",
                std::bind(&cc::check_journey, this, p::_1));
}

ev_key get_event_at(schedule const& sched, Connection const* con,
                    std::size_t stop_idx, event_type const ev_type) {
  verify(stop_idx < con->stops()->Length(), "stop not in range");
  auto const stop = con->stops()->Get(stop_idx);
  auto const station_idx =
      get_station(sched, stop->station()->id()->str())->index_;
  auto const ev_time = unix_to_motistime(
      sched, ev_type == event_type::DEP ? stop->departure()->schedule_time()
                                        : stop->arrival()->schedule_time());
  verify(ev_time != INVALID_TIME, "interchange event time not valid");

  auto const trip = std::find_if(
      std::begin(*con->trips()), std::end(*con->trips()),
      [&ev_type, &stop_idx](Trip const* trp) {
        return (ev_type == event_type::ARR &&
                trp->range()->to() == static_cast<uint16_t>(stop_idx)) ||
               (ev_type == event_type::DEP &&
                trp->range()->from() == static_cast<uint16_t>(stop_idx));
      });
  verify(trip != std::end(*con->trips()), "no trip ends/starts at interchange");
  auto const trp = from_fbs(sched, trip->id());

  auto const edge = std::find_if(
      begin(*trp->edges_), end(*trp->edges_), [&](trip::route_edge const& e) {
        auto const k = ev_key{e, trp->lcon_idx_, ev_type};
        auto const schedule_time = get_schedule_time(sched, k);
        return (ev_type == event_type::ARR &&
                e->to_->get_station()->id_ == station_idx &&
                schedule_time == ev_time) ||
               (ev_type == event_type::DEP &&
                e->from_->get_station()->id_ == station_idx &&
                schedule_time == ev_time);
      });
  verify(edge != end(*trp->edges_), "interchange event not in trip");

  return ev_key{*edge, trp->lcon_idx_, ev_type};
}

std::vector<interchange> get_interchanges(schedule const& sched,
                                          Connection const* con) {
  std::vector<interchange> evs;

  interchange ic;
  auto stop_idx = 0;
  for (auto const& s : *con->stops()) {
    if (s->enter()) {
      ic.enter_ = get_event_at(sched, con, stop_idx, event_type::DEP);
      ic.enter_stop_idx_ = stop_idx;
      if (ic.leave_.valid()) {
        evs.emplace_back(std::move(ic));
      }
    }

    if (s->leave()) {
      ic.reset();
      ic.leave_ = get_event_at(sched, con, stop_idx, event_type::ARR);
      ic.leave_stop_idx_ = stop_idx;
    }

    ++stop_idx;
  }

  return evs;
}

motis::time get_foot_edge_duration(schedule const& sched, Connection const* con,
                                   std::size_t src_stop_idx) {
  verify(src_stop_idx + 1 < con->stops()->Length(),
         "walk target index out of range");

  auto const from = get_station(
      sched, con->stops()->Get(src_stop_idx)->station()->id()->str());
  auto const to = get_station(
      sched, con->stops()->Get(src_stop_idx + 1)->station()->id()->str());

  auto const from_node = sched.station_nodes_.at(from->index_).get();
  auto const to_node = sched.station_nodes_.at(to->index_).get();

  verify(from_node->foot_node_ != nullptr, "walk src node has no foot node");
  auto const& foot_edges = from_node->foot_node_->edges_;
  auto const fe_it =
      std::find_if(begin(foot_edges), end(foot_edges),
                   [](edge const& e) { return e.to_ == to_node; });
  verify(fe_it != end(foot_edges), "foot edge not found");

  return fe_it->get_minimum_cost().time_;
}

void check_interchange(schedule const& sched, Connection const* con,
                       interchange const& ic) {
  auto const transfer_time = ic.enter_.get_time() - ic.leave_.get_time();
  if (ic.leave_stop_idx_ == ic.enter_stop_idx_) {
    verify(transfer_time >=
               sched.stations_.at(ic.enter_.get_station_idx())->transfer_time_,
           "transfer time below station transfer time");
  } else {
    auto const min_transfer_time = 0;
    for (auto i = ic.leave_stop_idx_; i < ic.enter_stop_idx_; ++i) {
      min_transfer_time += get_foot_edge_duration(sched, con, i);
    }
    verify(transfer_time >= min_transfer_time,
           "transfer below walk tranfer time");
  }
}

msg_ptr cc::check_journey(msg_ptr const& msg) const {
  auto const con = motis_content(Connection, msg);
  auto const& sched = get_schedule();
  for (auto const& ic : get_interchanges(sched, con)) {
    check_interchange(sched, con, ic);
  }
  return msg;
}

}  // namespace cc
}  // namespace motis
