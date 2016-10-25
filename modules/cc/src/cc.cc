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
  interchange() = default;
  interchange(ev_key leave, ev_key enter)
      : leave_(std::move(leave)), enter_(std::move(enter)) {}
  void reset() {
    enter_ = ev_key();
    leave_ = ev_key();
  }
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
      if (ic.leave_.valid()) {
        evs.emplace_back(std::move(ic));
      }
    }

    if (s->leave()) {
      ic.reset();
      ic.leave_ = get_event_at(sched, con, stop_idx, event_type::ARR);
    }

    ++stop_idx;
  }

  return evs;
}

void check_interchange(interchange const& ic) {
  if (ic.leave_.get_station_idx() != ic.enter_.get_station_idx()) {
    // TODO(felix) check walk times
  } else {
    // TODO(felix) check interchange times
  }
}

msg_ptr cc::check_journey(msg_ptr const& msg) const {
  auto const con = motis_content(Connection, msg);
  for (auto const& ic : get_interchanges(get_schedule(), con)) {
    check_interchange(ic);
  }
  return msg;
}

}  // namespace cc
}  // namespace motis
