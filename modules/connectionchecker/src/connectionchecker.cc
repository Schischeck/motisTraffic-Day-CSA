#include "motis/connectionchecker/connectionchecker.h"

#include <iostream>
#include <fstream>
#include <string>

#include "boost/program_options.hpp"

#include "motis/core/journey/journey.h"
#include "motis/core/journey/journey_util.h"
#include "motis/core/journey/message_to_journeys.h"
#include "motis/core/journey/journeys_to_message.h"

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

void connectionchecker::on_msg(msg_ptr msg, sid, callback cb) {
  auto journeys = message_to_journeys(msg->content<RoutingResponse const*>());

  MessageCreator b;
  std::vector<Offset<RealtimeTrainInfoRequest>> events;

  auto sync = synced_sched<schedule_access::RO>();
  auto const& eva_to_station = sync.sched().eva_to_station;
  auto schedule_begin = sync.sched().schedule_begin_;

  for (auto const& journey : journeys) {
    foreach_light_connection(journey, [&](journey::transport const& transport,
                                          journey::stop const& tail_stop,
                                          journey::stop const& head_stop) {
      auto tail_station = eva_to_station.find(tail_stop.eva_no);
      if (tail_station == end(eva_to_station)) {
        return cb({}, error::failure);
      }
      auto departure_req = CreateGraphTrainEvent(
          b, transport.train_nr, tail_station->second->index, true,
          unix_to_motistime(schedule_begin, tail_stop.departure.timestamp),
          transport.route_id);
      events.push_back(CreateRealtimeTrainInfoRequest(b, departure_req, true));

      auto head_station = eva_to_station.find(head_stop.eva_no);
      if (head_station == end(eva_to_station)) {
        return cb({}, error::failure);
      }
      auto arrival_req = CreateGraphTrainEvent(
          b, transport.train_nr, head_station->second->index, false,
          unix_to_motistime(schedule_begin, head_stop.arrival.timestamp),
          transport.route_id);
      events.push_back(CreateRealtimeTrainInfoRequest(b, arrival_req, true));
    });
  }

  b.CreateAndFinish(
      MsgContent_RealtimeTrainInfoBatchRequest,
      CreateRealtimeTrainInfoBatchRequest(b, b.CreateVector(events)).Union());

  auto mutate_and_reply = [journeys, schedule_begin, cb](
      msg_ptr res, error_code ec) mutable {
    if (ec) {
      return cb({}, ec);
    }
    auto infos = res->content<RealtimeTrainInfoBatchResponse const*>();
    auto it = infos->trains()->begin();

    for (auto& journey : journeys) {
      foreach_light_connection(
          journey, [&](journey::transport const&, journey::stop& tail_stop,
                       journey::stop& head_stop) {
            assert(it != infos->trains()->end());
            tail_stop.departure.schedule_timestamp = motis_to_unixtime(
                schedule_begin, (*it)->stops()->Get(0)->scheduled_time());
            ++it;

            assert(it != infos->trains()->end());
            head_stop.arrival.schedule_timestamp = motis_to_unixtime(
                schedule_begin, (*it)->stops()->Get(0)->scheduled_time());
            ++it;
          });
    }
    return cb(journeys_to_message(journeys), error::ok);
  };
  dispatch(make_msg(b), 0, mutate_and_reply);
}

}  // namespace connectionchecker
}  // namespace motis
