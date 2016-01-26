#include "motis/railviz/realtime_response.h"

namespace motis {
namespace railviz {

realtime_response::realtime_response(motis::module::msg_ptr msg)
    : msg(msg), rsp(nullptr), batch_rsp(nullptr) {
  auto const& parsed_msg = msg->get();
  if (parsed_msg->content_type() == MsgContent_RealtimeTrainInfoResponse) {
    rsp = reinterpret_cast<motis::realtime::RealtimeTrainInfoResponse const*>(
        parsed_msg->content());
  } else if (parsed_msg->content_type() ==
             MsgContent_RealtimeTrainInfoBatchResponse) {
    batch_rsp = reinterpret_cast<
        motis::realtime::RealtimeTrainInfoBatchResponse const*>(
        parsed_msg->content());
  }
}

std::pair<unsigned int, unsigned int> realtime_response::delay(
    const light_connection& lc, const edge& e) const {
  std::pair<unsigned int, unsigned int> d = {0, 0};
  d.first = delay(lc, *e._from->get_station(), true, e._from->_route);
  d.second = delay(lc, *e._to->get_station(), false, e._to->_route);
  return d;
}

unsigned int realtime_response::delay(const timetable_entry& te) const {
  return delay(*std::get<0>(te), *std::get<1>(te), std::get<4>(te),
               std::get<5>(te));
}

std::pair<unsigned int, unsigned int> realtime_response::delay(
    const route_entry& route_entry_) const {
  if (std::get<2>(route_entry_) == nullptr) return {0, 0};

  std::pair<unsigned int, unsigned int> delays = {0, 0};
  const motis::station_node* next_station = next_station_node(
      *std::get<1>(route_entry_), std::get<1>(route_entry_)->_route);
  delays.first = delay(*std::get<2>(route_entry_), *std::get<0>(route_entry_),
                       true, std::get<1>(route_entry_)->_route);
  delays.second = delay(*std::get<2>(route_entry_), *next_station, false,
                        std::get<1>(route_entry_)->_route);

  return delays;
}

unsigned int realtime_response::delay(const light_connection& lc,
                                      const station_node& station_node_,
                                      bool departure,
                                      unsigned int route_id) const {
  if (rsp != nullptr) {
    return delay_single(*rsp, lc, station_node_, departure);
  } else if (batch_rsp != nullptr) {
    return delay_batch(lc, station_node_, departure, route_id);
  }
  return 0;
}

unsigned int realtime_response::delay_batch(const light_connection& lc,
                                            const station_node& station_node_,
                                            bool departure,
                                            unsigned int route_id) const {
  if (batch_rsp == nullptr) {
    return 0;
  }

  int num_responses = batch_rsp->trains()->Length();
  for (int i = 0; i < num_responses; i++) {
    realtime::RealtimeTrainInfoResponse const* response =
        batch_rsp->trains()->Get(i);
    if (response->route_id() == route_id) {
      int delay = delay_single(*response, lc, station_node_, departure);
      if (delay != 0) return delay;
    }
  }

  return 0;
}

unsigned int realtime_response::delay_single(
    const motis::realtime::RealtimeTrainInfoResponse& response,
    const light_connection& lc, const station_node& station_node_,
    bool departure) const {
  int num_stops = response.stops()->Length();
  for (int i = 0; i < num_stops; i++) {
    const realtime::EventInfo* event_info = response.stops()->Get(i);
    unsigned int real_time = departure ? lc.d_time : lc.a_time;
    if (event_info->departure() == departure &&
        event_info->real_time() == real_time &&
        event_info->train_nr() == lc._full_con->con_info->train_nr &&
        event_info->station_index() == station_node_._id) {
      return event_info->real_time() - event_info->scheduled_time();
    }
  }

  return 0;
}

const motis::station_node* realtime_response::next_station_node(
    const motis::node& node_, unsigned int route_id) const {
  for (const motis::edge& e : node_._edges) {
    if (e.type() == motis::edge::ROUTE_EDGE) {
      if (e._to->is_route_node() &&
          static_cast<unsigned>(e._to->_route) == route_id)
        return e._to->get_station();
    }
  }
  return nullptr;
}
}
}
