#include <ctime>
#include <algorithm>
#include <memory>
#include <iostream>
#include <motis/realtime/statistics.h>

#include "boost/lexical_cast.hpp"

#include "motis/core/common/logging.h"
#include "motis/core/schedule/connection.h"
#include "motis/realtime/message_handler.h"
#include "motis/realtime/realtime_schedule.h"
#include "motis/realtime/modified_train.h"

namespace motis {
namespace realtime {

using namespace motis::logging;

message_handler::message_handler(realtime_schedule& rts)
    : _rts(rts), _msg_reader(rts) {}

void message_handler::process_message_stream(message_stream& stream) {
  unsigned messages = 0;
  std::vector<std::unique_ptr<message>> msg_objects;
  std::time_t max_release_time = 0;

  while (true) {
    std::unique_ptr<message> msg = stream.next_message();
    if (msg == nullptr) break;
    handle_message(*msg);
    messages++;
    max_release_time = std::max(max_release_time, msg->release_time_);
    msg_objects.push_back(std::move(msg));
  }
  if (messages != 0) {
    _rts._delay_propagator.process_queue();
    _rts._graph_updater.finish_graph_update();
    _rts._message_output.set_current_time(max_release_time);
    _rts._message_output.finish();
  }
}

void message_handler::handle_message(const message& msg) {
  _rts._stats._counters.messages.increment();
  switch (msg.type_) {
    case message_type::delay:
      handle_delay(static_cast<const delay_message&>(msg));
      break;
    case message_type::additional:
      handle_additional_train(
          static_cast<const additional_train_message&>(msg));
      _rts._message_output.add_message(&msg);
      break;
    case message_type::cancel:
      handle_canceled_train(static_cast<const cancel_train_message&>(msg));
      _rts._message_output.add_message(&msg);
      break;
    case message_type::reroute:
      handle_rerouted_train(static_cast<const reroute_train_message&>(msg));
      //_rts._message_output.add_message(&msg); // TODO:
      // reroute_train_message_s_g
      break;
    case message_type::status_decision:
      handle_connection_status_decision(
          static_cast<const connection_status_decision_message&>(msg));
      _rts._message_output.add_message(&msg);
      break;
    default:
      _rts._stats._counters.unknown.increment();
      _rts._stats._counters.unknown.ignore();
      break;
  }
}

void message_handler::handle_delay(const schedule_event& schedule_event,
                                   motis::time new_time,
                                   timestamp_reason reason) {
  _rts._stats._counters.delay_events.increment();
  if (reason == timestamp_reason::IS) {
    _rts._stats._counters.delay_is.increment();
  } else if (reason == timestamp_reason::FORECAST) {
    _rts._stats._counters.delay_fc.increment();
  }

  if (std::abs(new_time - schedule_event._schedule_time) > MINUTES_A_DAY) {
    LOG(warn) << "invalid delay message received for " << schedule_event
              << ", new_time=" << motis::format_time(new_time)
              << ", reason=" << reason << " - ignoring message";

    _rts._stats._counters.delay_events.ignore();
    if (reason == timestamp_reason::IS) {
      _rts._stats._counters.delay_is.ignore();
    } else if (reason == timestamp_reason::FORECAST) {
      _rts._stats._counters.delay_fc.ignore();
    }
    return;
  }

  _rts._delay_propagator.handle_delay_message(schedule_event, new_time, reason);
}

void message_handler::handle_delay(const delay_message& msg) {
  bool old_debug = _rts._debug_mode;
  if (_rts.is_tracked(msg.train_nr_)) {
    _rts._tracking.in_message(msg);
    _rts._debug_mode = true;
  }
  _rts._stats._counters.delay_msgs.increment();
  if (msg.is_.valid() && msg.is_.delayed_time_ != motis::INVALID_TIME) {
    handle_delay(msg.is_, msg.is_.delayed_time_, timestamp_reason::IS);
  }
  for (const delayed_event& delayed_stop : msg.forecasts_) {
    if (delayed_stop.valid() &&
        delayed_stop.delayed_time_ != motis::INVALID_TIME) {
      handle_delay(delayed_stop, delayed_stop.delayed_time_,
                   timestamp_reason::FORECAST);
    }
  }
  _rts._debug_mode = old_debug;
}

void message_handler::handle_additional_train(
    const additional_train_message& msg) {
  _rts._stats._counters.additional.increment();

  if (_rts.is_tracked(msg.train_nr_)) {
    _rts._tracking.in_message(msg);
  }

  if (msg.events_.empty()) {
    if (_rts.is_debug_mode())
      LOG(debug) << "skipping empty additional train message";
    _rts._stats._counters.additional.ignore();
    return;
  }

  if (!is_valid_train(msg.events_)) {
    if (_rts.is_debug_mode())
      LOG(debug) << "skipping additional train message, would result in broken "
                 << "train";
    _rts._stats._counters.additional.ignore();
    return;
  }

  // check if train already exists
  for (const schedule_event& e : msg.events_) {
    if (_rts.event_exists(e)) {
      LOG(warn) << "event included in additional train already exists in graph"
                   ", skipping message";
      _rts._stats._counters.additional.ignore();
      return;
    }
  }

  uint32_t category_index;
  uint8_t clasz;
  std::tie(category_index, clasz) = get_or_create_category(msg.category_);

  connection_info* ci = new connection_info();
  ci->train_nr = msg.events_[0]._train_nr;
  ci->family = category_index;
  ci->line_identifier = "";

  // family = category -> _schedule.category_names[ci->family]
  _rts._new_connection_infos.emplace_back(ci);

  int32_t route_id = ++_rts._max_route_id;
  modified_train* mt = new modified_train(route_id, route_id, ci, clasz);

  _rts._modified_train_manager.add(mt);
  _rts._graph_updater.adjust_train(mt, msg.events_);

  //  for (const auto& e : events) {
  //    _rts._delay_propagator.enqueue(e, queue_reason::RECALC,
  //    mt->_new_route_id);
  //  }
}

void message_handler::handle_canceled_train(const cancel_train_message& msg) {
  _rts._stats._counters.canceled.increment();
  if (_rts.is_tracked(msg.train_nr_)) {
    _rts._tracking.in_message(msg);
  }
  if (msg.events_.empty()) {
    _rts._stats._counters.canceled.ignore();
    return;
  }

  std::vector<std::tuple<node*, schedule_event, schedule_event>> current_events;
  modified_train* mt;
  for (schedule_event const& e : msg.events_) {
    std::tie(current_events, mt) =
        _rts._graph_updater.get_current_events_and_make_modified_train(e);
    if (mt != nullptr) break;
  }

  if (mt == nullptr) {
    // train not found
    if (_rts.is_debug_mode()) LOG(debug) << "canceled train not found";
    _rts._stats._counters.canceled.ignore();
    return;
  }

  // if first canceled event is an arrival, add the previous departure event
  // as well
  std::vector<schedule_event> canceled_events = msg.events_;
  if (canceled_events[0].arrival()) {
    schedule_event arrival = canceled_events[0];
    schedule_event departure =
        _rts.get_previous_schedule_event(_rts.get_graph_event(arrival));
    if (departure.found()) {
      LOG(debug) << "add canceled departure: " << departure
                 << " for arrival: " << arrival;
      canceled_events.push_back(departure);
    }
  }

  std::vector<schedule_event> remaining_events;

  for (const auto& e : current_events) {
    schedule_event arrival_event, departure_event;
    std::tie(std::ignore, arrival_event, departure_event) = e;
    if (arrival_event.found() &&
        std::find(canceled_events.begin(), canceled_events.end(),
                  arrival_event) == canceled_events.end())
      remaining_events.push_back(arrival_event);
    if (departure_event.found() &&
        std::find(canceled_events.begin(), canceled_events.end(),
                  departure_event) == canceled_events.end())
      remaining_events.push_back(departure_event);
  }

  if (_rts.is_debug_mode()) {
    LOG(debug) << "cancel_train - canceled events:";
    for (const auto& e : canceled_events) {
      LOG(debug) << "  " << e;
    }
    LOG(debug) << "cancel_train - remaining events:";
    for (const auto& e : remaining_events) {
      LOG(debug) << "  " << e;
    }
  }

  if (!is_valid_train(remaining_events)) {
    if (_rts.is_debug_mode())
      LOG(debug)
          << "skipping cancel train message, would result in broken train";
    // restore previous version (make_modified_train removes the train)
    std::vector<schedule_event> old_events;
    for (const auto& e : current_events) {
      schedule_event arrival_event, departure_event;
      std::tie(std::ignore, arrival_event, departure_event) = e;
      if (arrival_event.found()) old_events.push_back(arrival_event);
      if (departure_event.found()) old_events.push_back(departure_event);
    }
    _rts._graph_updater.adjust_train(mt, old_events);
    _rts._stats._counters.canceled.ignore();
    return;
  }

  _rts._graph_updater.adjust_train(mt, remaining_events);

  for (const auto& e : canceled_events) {
    delay_info* di =
        _rts._delay_info_manager.cancel_event(e, mt->_new_route_id);
    _rts._delay_propagator.enqueue(di, queue_reason::CANCELED);
  }
}

void message_handler::handle_rerouted_train(const reroute_train_message& msg) {
  _rts._stats._counters.reroutings.increment();
  if (_rts.is_tracked(msg.train_nr_)) {
    _rts._tracking.in_message(msg);
  }
  if (msg.canceled_events_.empty() && msg.new_events_.empty()) {
    _rts._stats._counters.reroutings.ignore();
    return;
  }

  modified_train* mt = nullptr;
  std::vector<std::tuple<node*, schedule_event, schedule_event>> current_events;

  if (_rts.is_debug_mode()) {
    LOG(debug) << "handle_rerouted_train: (category: " << msg.category_ << ")";
    LOG(debug) << "-- canceled_events:";
    for (auto& e : msg.canceled_events_) LOG(debug) << "---- " << e;
    LOG(debug) << "-- new_events:";
    for (auto& e : msg.new_events_) LOG(debug) << "---- " << e;
  }

  // try to find the train

  if (!msg.canceled_events_.empty()) {
    // try to find the train using the canceled events
    for (const schedule_event& e : msg.canceled_events_) {
      // if a modified train with this event already exists, it is returned.
      // otherwise, if the event is found, a new modified train is created.
      std::tie(current_events, mt) =
          _rts._graph_updater.get_current_events_and_make_modified_train(e);
      if (mt != nullptr) {
        if (_rts.is_debug_mode())
          LOG(debug) << "-- found MT using " << e << ", "
                     << current_events.size() << " current events";
        break;
      }
    }
  }

  if (mt == nullptr) {
    // if not found so far, try to find a departure event with the given train
    // number and day
    int day_index = 0;
    uint32_t train_nr;
    if (!msg.canceled_events_.empty()) {
      day_index = msg.canceled_events_[0]._schedule_time / MINUTES_A_DAY;
      train_nr = msg.canceled_events_[0]._train_nr;
    } else if (!msg.new_events_.empty()) {
      day_index = msg.new_events_[0]._schedule_time / MINUTES_A_DAY;
      train_nr = msg.new_events_[0]._train_nr;
    } else {
      if (_rts.is_debug_mode())
        LOG(warn) << "reroute: both event lists are empty, skipping";
      _rts._stats._counters.reroutings.ignore();
      return;
    }
    schedule_event ref_event = _rts.find_departure_event(train_nr, day_index);
    if (ref_event.found()) {
      if (_rts.is_debug_mode())
        LOG(info) << "reroute: found event for train " << train_nr << " on day "
                  << day_index << ": " << ref_event;
      std::tie(current_events, mt) =
          _rts._graph_updater.get_current_events_and_make_modified_train(
              ref_event);
    }
  }

  if (mt == nullptr) {
    // train not found
    if (_rts.is_debug_mode()) LOG(warn) << "reroute: train not found!";
    _rts._stats._counters.reroutings.ignore();
    return;
  }

  if (_rts.is_debug_mode())
    LOG(debug) << "-- mt: route_id=" << mt->_new_route_id
               << ", current_start_event=" << mt->_current_start_event
               << ", addr=" << mt;

  // build the new list of events
  std::vector<schedule_event> all_events;
  // and a list of events that are actually canceled
  std::vector<schedule_event> removed_events;

  if (_rts.is_debug_mode())
    LOG(debug) << "building all_events using " << current_events.size()
               << " current events and " << msg.new_events_.size()
               << " new events";

  // keep current events that are not canceled
  for (const auto& ce : current_events) {
    schedule_event arr, dep;
    std::tie(std::ignore, arr, dep) = ce;

    if (arr.found()) {
      if (std::find(msg.canceled_events_.begin(), msg.canceled_events_.end(),
                    arr) == msg.canceled_events_.end())
        all_events.push_back(arr);
      else
        removed_events.push_back(arr);
    }

    if (dep.found()) {
      if (std::find(msg.canceled_events_.begin(), msg.canceled_events_.end(),
                    dep) == msg.canceled_events_.end())
        all_events.push_back(dep);
      else
        removed_events.push_back(dep);
    }
  }

  // add the new events
  for (const schedule_event& ne : msg.new_events_) {
    // ignore event if it is already in the list
    if (std::find(all_events.begin(), all_events.end(), ne) != all_events.end())
      continue;
    graph_event ge;
    if (_rts.event_exists(ne, &ge)) {
      if (ge._route_id != mt->_new_route_id) {
        LOG(warn)
            << "new event included in rerouted train already exists in graph: "
            << ne << ": " << _rts.get_graph_event(ne) << " / " << ge;
        // TODO: restore previous train
        break;
      }
    }
    const auto it = std::upper_bound(all_events.begin(), all_events.end(), ne);
    if (it == all_events.end() || *it > ne) {
      all_events.insert(it, ne);
    }
  }

  if (_rts.is_debug_mode()) {
    LOG(debug) << "new event list (all_events):";
    for (auto& e : all_events) LOG(debug) << "-- " << e;
  }

  // check that the event list is consistent
  if (!is_valid_train(all_events)) {
    if (_rts.is_debug_mode())
      LOG(debug) << "skipping reroute message, would result in broken train";
    // restore previous version (make_modified_train removes the train)
    std::vector<schedule_event> old_events;
    for (const auto& e : current_events) {
      schedule_event arrival_event, departure_event;
      std::tie(std::ignore, arrival_event, departure_event) = e;
      if (arrival_event.found()) old_events.push_back(arrival_event);
      if (departure_event.found()) old_events.push_back(departure_event);
    }
    _rts._graph_updater.adjust_train(mt, old_events);
    _rts._stats._counters.reroutings.ignore();
    return;
  }

  bool update_all_edges = false;
  if (msg.category_ !=
      _rts._schedule.category_names[mt->_connection_info->family]) {
    // category change
    if (_rts.is_debug_mode())
      LOG(debug) << "reroute: category change: "
                 << _rts._schedule.category_names[mt->_connection_info->family]
                 << " -> " << msg.category_;
    uint32_t category_index;
    uint8_t clasz;
    std::tie(category_index, clasz) = get_or_create_category(msg.category_);
    mt->_connection_info->family = category_index;
    mt->_clasz = clasz;
    update_all_edges = true;
  }

  if (_rts.is_debug_mode()) {
    LOG(debug) << "removed events:";
    for (auto& e : removed_events) LOG(debug) << "-- " << e;
  }

  _rts._graph_updater.adjust_train(mt, all_events, update_all_edges);

  for (const auto& e : removed_events) {
    delay_info* di =
        _rts._delay_info_manager.cancel_event(e, mt->_new_route_id);
    _rts._delay_propagator.enqueue(di, queue_reason::CANCELED);
  }
  for (const auto& e : all_events) {
    delay_info* di = _rts._delay_info_manager.undo_cancelation(e);
    if (di != nullptr) _rts._delay_propagator.enqueue(di, queue_reason::RECALC);
  }
}

void message_handler::handle_connection_status_decision(
    const connection_status_decision_message& msg) {
  _rts._stats._counters.csd.increment();
  if (_rts.is_tracked(msg.arrival_._train_nr) ||
      _rts.is_tracked(msg.departure_._train_nr)) {
    _rts._tracking.in_message(msg);
  }
  if (!msg.arrival_.valid() || !msg.departure_.valid()) {
    _rts._stats._counters.csd.ignore();
    return;
  }
  if (msg.decision_ == status_decision::kept) {
    _rts._waiting_edges.add_additional_edge(msg.arrival_, msg.departure_,
                                            std::numeric_limits<int>::max());
  } else {
    _rts._waiting_edges.remove_additional_edge(msg.arrival_, msg.departure_);
  }
  _rts._delay_propagator.enqueue(msg.arrival_, queue_reason::RECALC);
  _rts._delay_propagator.enqueue(msg.departure_, queue_reason::RECALC);
}

std::pair<uint32_t, uint8_t> message_handler::get_or_create_category(
    std::string category) {
  uint32_t category_index;
  uint8_t clasz = 9;  // default class
  auto it = std::find(std::begin(_rts._schedule.category_names),
                      std::end(_rts._schedule.category_names), category);
  if (it != std::end(_rts._schedule.category_names)) {
    category_index =
        std::distance(std::begin(_rts._schedule.category_names), it);
    auto it2 = _rts._schedule.classes.find(category);
    if (it2 != std::end(_rts._schedule.classes)) clasz = it2->second;
  } else {
    // category not found - create new category
    _rts._schedule.category_names.push_back(category);
    _rts._schedule.classes[category] = clasz;
    category_index = _rts._schedule.category_names.size() - 1;
  }
  return {category_index, clasz};
}

bool message_handler::is_valid_train(
    const std::vector<schedule_event>& events) const {
  if (events.size() % 2 != 0) return false;

  for (std::size_t i = 0; i + 1 < events.size(); i += 2) {
    const schedule_event& dep = events[i];
    const schedule_event& arr = events[i + 1];
    if (!dep.departure() || !arr.arrival()) {
      if (_rts.is_debug_mode())
        LOG(warn) << "invalid new event list (departure/arrival)";
      return false;
    }
    if (i > 0 && dep._station_index != events[i - 1]._station_index) {
      if (_rts.is_debug_mode())
        LOG(warn) << "invalid new event list (stations)";
      return false;
    }
    if (dep._station_index == arr._station_index) {
      if (_rts.is_debug_mode()) {
        LOG(warn) << "invalid new event list (departure == arrival)";
      }
      return false;
    }
    if (arr._schedule_time < dep._schedule_time) {
      if (_rts.is_debug_mode()) LOG(warn) << "invalid times";
      return false;
    }
    if (!dep.found() || !arr.found()) {
      if (_rts.is_debug_mode()) LOG(warn) << "missing stations";
      return false;
    }
  }

  return true;
}

bool message_handler::train_exists(
    const std::vector<schedule_event>& events) const {
  assert(!events.empty());

  schedule_event start_event;
  std::tie(start_event, std::ignore, std::ignore, std::ignore) =
      _rts.locate_start_of_train(events[0]);
  if (!start_event.found()) return false;
  if (start_event != events[0]) {
    if (_rts.is_debug_mode()) LOG(warn) << "partial train match (v1)";
    return false;
  }

  std::vector<schedule_event> found_events;
  auto es = _rts.get_train_events(start_event);
  for (const auto& e : es) {
    schedule_event arrival, departure;
    std::tie(std::ignore, arrival, departure) = e;
    if (arrival.found()) found_events.push_back(arrival);
    if (departure.found()) found_events.push_back(arrival);
  }
  if (events == found_events) {
    return true;
  } else {
    if (_rts.is_debug_mode()) LOG(warn) << "partial train match (v2)";
    return false;
  }
}

}  // namespace realtime
}  // namespace motis
