#include <iostream>
#include <algorithm>
#include <tuple>

#include "motis/core/common/logging.h"
#include "motis/realtime/delay_propagator.h"
#include "motis/realtime/realtime_schedule.h"
#include "motis/realtime/statistics.h"

namespace motis {
namespace realtime {

using namespace motis::logging;

void delay_propagator::handle_delay_message(const schedule_event& event_id,
                                            motis::time new_time,
                                            timestamp_reason reason) {
  operation_timer timer(_rts._stats._delay_propagator);

  bool old_debug = _rts._debug_mode;
  if (_rts.is_tracked(event_id._train_nr)) {
    _rts._debug_mode = true;
  }
  if (_rts.is_debug_mode()) {
    LOG(debug) << "handle_delay_message: " << event_id
               << ", new_time = " << format_time(new_time)
               << ", reason = " << reason;
  }

  delay_info* di = _rts._delay_info_manager.get_delay_info(event_id);
  bool delay_info_exists = di != nullptr;
  if (di == nullptr) {
    node* route_node;
    std::tie(route_node, std::ignore) =
        _rts.locate_event(graph_event(event_id));
    if (route_node == nullptr) {
      if (_rts.is_debug_mode()) LOG(debug) << "event not found: " << event_id;
      _rts._debug_mode = old_debug;
      return;
    }
    di = _rts._delay_info_manager.create_delay_info(event_id,
                                                    route_node->_route);
  }

  if (_rts.is_debug_mode()) {
    LOG(debug) << "--delay_info = " << *di
               << (delay_info_exists ? " (existing)" : " (new)");
  }

  if (reason == timestamp_reason::IS) {
    add_update(di, new_time, reason);
    enqueue(di, queue_reason::IS);
  } else if (reason == timestamp_reason::REPAIR) {
    add_update(di, new_time, reason);
    enqueue(di, queue_reason::REPAIR);
  } else if (reason == timestamp_reason::FORECAST) {
    di->_forecast_time = new_time;
    enqueue(di, queue_reason::FORECAST);
  }
  _rts._debug_mode = old_debug;
}

void delay_propagator::process_queue() {
  if (_queue.empty()) return;
  operation_timer timer(_rts._stats._delay_propagator);
  _rts._stats._ops.propagator.runs++;

  if (_rts.is_debug_mode()) LOG(debug) << "processing queue...";
  while (!_queue.empty()) {
    auto it = _queue.begin();
    const delay_queue_entry& entry = *it;

    bool old_debug = _rts._debug_mode;
    if (_rts.is_tracked(entry._delay_info->_schedule_event._train_nr)) {
      _rts._debug_mode = true;
    }

    if (_rts.is_debug_mode()) {
      LOG(debug) << "** unqueued: (" << entry._queue_reason << ") "
                 << *entry._delay_info
                 << " - remaining queue size: " << _queue.size();
    }

    _rts._stats._ops.propagator.events++;

    bool event_updated;

    if (entry._queue_reason != queue_reason::IS &&
        entry._queue_reason != queue_reason::REPAIR &&
        entry._queue_reason != queue_reason::CANCELED) {
      // recalculate maximum
      event_updated = calculate_max(entry);
    } else {
      event_updated = true;
    }

    if (event_updated) {
      if (entry._queue_reason != queue_reason::CANCELED) {
        _rts._message_output.add_delay(entry._delay_info);
      }
      queue_dependent_events(entry);
    }

    // Repair invalid timestamps

    if (!entry._delay_info->canceled()) {
      motis::time this_time = new_time(entry._delay_info);
      timestamp_reason this_reason = new_reason(entry._delay_info);
      schedule_event next_event =
          _rts.get_next_schedule_event(entry._delay_info->graph_ev());
      if (next_event.found()) {
        delay_info* next_di =
            _rts._delay_info_manager.get_delay_info(next_event);
        if (next_di != nullptr) {
          motis::time next_time = new_time(next_di);
          timestamp_reason next_reason = new_reason(next_di);
          if (this_time > next_time &&
              (next_reason == timestamp_reason::IS ||
               next_reason == timestamp_reason::REPAIR)) {

            if (this_reason == timestamp_reason::IS ||
                this_reason == timestamp_reason::REPAIR) {
              if (_rts.is_debug_mode()) {
                LOG(info) << "Repair timestamp using repair message (forward): "
                          << *entry._delay_info << ": "
                          << motis::format_time(this_time) << " " << this_reason
                          << " => " << motis::format_time(next_time);
              }
              handle_delay_message(entry._delay_info->sched_ev(), next_time,
                                   timestamp_reason::REPAIR);
            }
          }
        }
      }

      if (this_reason == timestamp_reason::IS ||
          this_reason == timestamp_reason::REPAIR) {
        schedule_event prev_event =
            _rts.get_previous_schedule_event(entry._delay_info->graph_ev());
        if (prev_event.found()) {
          motis::time prev_time = new_time(prev_event);
          if (this_time < prev_time) {
            if (_rts.is_debug_mode()) {
              LOG(info) << "Repair timestamp using repair message (backward): "
                        << prev_event << ": " << motis::format_time(prev_time)
                        << " => " << motis::format_time(this_time);
            }
            handle_delay_message(prev_event, this_time,
                                 timestamp_reason::REPAIR);
          }
        }
      }
    }

    _queue.erase(it);
    _rts._debug_mode = old_debug;
  }
  if (_rts.is_debug_mode()) LOG(debug) << "all delays processed";
  apply_updates();
}

bool delay_propagator::calculate_max(const delay_queue_entry& entry) {
  operation_timer timer(_rts._stats._calc_max);
  delay_info* di = entry._delay_info;

  if (_rts.is_debug_mode())
    LOG(debug) << "calculate_max: " << *di << " (new reason: " << new_reason(di)
               << ", new time: " << motis::format_time(new_time(di)) << ")";

  auto check_reason = new_reason(di);
  if (check_reason == timestamp_reason::IS ||
      check_reason == timestamp_reason::REPAIR) {
    return false;
  }

  _rts._stats._ops.propagator.calc_max++;

  // schedule time
  motis::time max = di->sched_ev()._schedule_time;
  timestamp_reason reason = timestamp_reason::SCHEDULE;

  // forecast time
  motis::time forecast_time = di->_forecast_time;
  if (forecast_time != INVALID_TIME && forecast_time > max) {
    max = forecast_time;
    reason = timestamp_reason::FORECAST;
  }

  schedule_event prev_event = _rts.get_previous_schedule_event(di->graph_ev());
  if (di->sched_ev().departure()) {  // departure event
    // standing edge
    if (prev_event.found()) {
      motis::time prev_arrival_time = new_time(prev_event);
      int scheduled_standing =
          di->sched_ev()._schedule_time - prev_event._schedule_time;
      // TODO: constant/parameter for min standing time (2)
      motis::time standing =
          prev_arrival_time + std::min(scheduled_standing, 2);
      if (standing > max) {
        max = standing;
        reason = timestamp_reason::PROPAGATION;
      }
    }

    // waiting edges
    for (const single_waiting_edge& we :
         _rts._waiting_edges.get_edges_to(di->sched_ev(), di->_route_id)) {
      delay_info* feeder_di =
          _rts._delay_info_manager.get_delay_info(we._feeder_arrival);
      if (feeder_di == nullptr || feeder_di->canceled()) {
        continue;
      }

      motis::time feeder_arrival_time = new_time(feeder_di);
      if (feeder_arrival_time == we._feeder_arrival._schedule_time) {
        continue;
      }
      int ic =
          _rts._schedule.stations[di->sched_ev()._station_index]->transfer_time;
      if (we._waiting_time == std::numeric_limits<int>::max() ||
          feeder_arrival_time + ic <=
              di->sched_ev()._schedule_time + we._waiting_time) {
        motis::time wait_until = feeder_arrival_time + ic;
        if (wait_until > max) {
          max = wait_until;
          reason = timestamp_reason::PROPAGATION;
        }
      }
    }

  } else if (prev_event.found()) {  // arrival event
    // traveling edge
    motis::time prev_departure_time = new_time(prev_event);
    int scheduled_edge_length =
        di->sched_ev()._schedule_time - prev_event._schedule_time;
    motis::time traveling = prev_departure_time + scheduled_edge_length;
    if (traveling > max) {
      max = traveling;
      reason = timestamp_reason::PROPAGATION;
    }
  }

  if (max != di->_current_time) {
    add_update(di, max, reason);
    return true;
  } else {
    return false;
  }
}

void delay_propagator::queue_dependent_events(const delay_queue_entry& entry) {
  operation_timer timer(_rts._stats._queue_dep);
  delay_info* di = entry._delay_info;

  if (_rts.is_debug_mode()) LOG(debug) << "queue_dependent_events: " << *di;

  // next event of this train
  schedule_event next_event = _rts.get_next_schedule_event(di->graph_ev());
  if (next_event.found())
    enqueue(next_event, queue_reason::TRAIN, di->_route_id);

  bool is_delayed = new_time(di) != di->sched_ev()._schedule_time;
  bool was_delayed = di->delayed();

  // waiting edges
  if (di->_schedule_event.departure()) return;
  for (const single_waiting_edge& we :
       _rts._waiting_edges.get_edges_from(di->sched_ev(), di->_route_id)) {
    delay_info* connector_di =
        _rts._delay_info_manager.get_delay_info(we._connector_departure);

    // check if we have to queue the event
    if (connector_di != nullptr &&
        (connector_di->_reason == timestamp_reason::IS ||
         connector_di->_reason == timestamp_reason::REPAIR)) {
      continue;  // event already has is message
    }

    // calc if connector would wait
    if (is_delayed && !entry._delay_info->canceled()) {
      int ic =
          _rts._schedule.stations[di->sched_ev()._station_index]->transfer_time;
      motis::time wait_time = new_time(di) + ic;
      bool would_wait = (we._connector_departure._schedule_time < wait_time) &&
                        (wait_time <= we._connector_departure._schedule_time +
                                          we._waiting_time);
      if (would_wait &&
          (connector_di == nullptr ||
           (connector_di->_reason != timestamp_reason::PROPAGATION ||
            connector_di->_current_time != wait_time))) {
        if (connector_di != nullptr)
          enqueue(connector_di, queue_reason::WAITING);
        else
          enqueue(we._connector_departure, queue_reason::WAITING);
      }
    }

    if (connector_di != nullptr &&
        connector_di->_reason == timestamp_reason::PROPAGATION &&
        (is_delayed || was_delayed || entry._delay_info->canceled())) {
      enqueue(connector_di, queue_reason::WAITING);
    }
  }
}

void delay_propagator::add_update(delay_info* di, motis::time new_time,
                                  timestamp_reason new_reason) {
  if (_rts.is_debug_mode()) {
    LOG(debug) << "==> add_update: " << *di
               << ", new_time = " << format_time(new_time)
               << ", new_reason = " << new_reason;
  }

  _rts._stats._ops.propagator.updates++;

  assert(di->_route_id > 0);

  std::vector<delay_info_update>& updates = _delay_info_updates[di->_route_id];
  auto e = std::find_if(
      updates.begin(), updates.end(),
      [di](const delay_info_update& u) { return u._delay_info == di; });
  if (e != updates.end()) {
    e->_new_time = new_time;
    e->_new_reason = new_reason;
  } else {
    updates.emplace_back(di, new_time, new_reason);
  }
}

void delay_propagator::apply_updates() {
  if (_rts.is_debug_mode())
    LOG(debug) << "============ applying updates ============";
  for (auto p : _delay_info_updates) {
    std::vector<delay_info_update>& updates = p.second;
    _rts._graph_updater.perform_updates(updates);
  }
  _delay_info_updates.clear();
  if (_rts.is_debug_mode())
    LOG(debug) << "============ updates applied ============";
}

void delay_propagator::enqueue(const schedule_event& event_id,
                               queue_reason queue_reason, int32_t route_id) {
  delay_info* di = _rts._delay_info_manager.get_delay_info(event_id);
  if (di == nullptr) {
    if (route_id == -1) {
      node* route_node = _rts.locate_event(graph_event(event_id)).first;
      if (route_node == nullptr) return;
      route_id = route_node->_route;
    }
    di = _rts._delay_info_manager.create_delay_info(event_id, route_id);
  } else if (route_id != -1) {
    di->_route_id = route_id;
  }
  assert(di->_route_id > 0);
  enqueue(di, queue_reason);
}

void delay_propagator::enqueue(delay_info* di, queue_reason queue_reason) {
  assert(di != nullptr);
  if (_rts.is_debug_mode())
    LOG(debug) << "enqueue: " << *di << ", queue_reason = " << queue_reason;
  _queue.emplace(di, queue_reason);
}

void delay_propagator::update_route(delay_info* di, int32_t new_route) {
  assert(di->_route_id > 0);
  assert(di->_route_id != new_route);

  std::vector<delay_info_update>& old_route_updates =
      _delay_info_updates[di->_route_id];
  auto e = std::find_if(
      old_route_updates.begin(), old_route_updates.end(),
      [di](const delay_info_update& u) { return u._delay_info == di; });
  if (e != old_route_updates.end()) {
    _delay_info_updates[new_route].emplace_back(*e);
    old_route_updates.erase(e);
  }
}

motis::time delay_propagator::new_time(delay_info* di) const {
  assert(di->_route_id > 0);
  auto it = _delay_info_updates.find(di->_route_id);
  if (it != _delay_info_updates.end()) {
    auto& updates = it->second;
    auto u = std::find_if(
        updates.begin(), updates.end(),
        [di](const delay_info_update& u) { return u._delay_info == di; });
    if (u != updates.end()) return u->_new_time;
  }
  return di->_current_time;
}

motis::time delay_propagator::new_time(const schedule_event& event_id) const {
  delay_info* di = _rts._delay_info_manager.get_delay_info(event_id);
  if (di != nullptr)
    return new_time(di);
  else
    return event_id._schedule_time;
}

timestamp_reason delay_propagator::new_reason(delay_info* di) const {
  assert(di->_route_id > 0);
  auto it = _delay_info_updates.find(di->_route_id);
  if (it != _delay_info_updates.end()) {
    auto& updates = it->second;
    auto u = std::find_if(
        updates.begin(), updates.end(),
        [di](const delay_info_update& u) { return u._delay_info == di; });
    if (u != updates.end()) return u->_new_reason;
  }
  return di->_reason;
}

std::ostream& operator<<(std::ostream& os, const queue_reason& r) {
  switch (r) {
    case queue_reason::IS: os << "is"; break;
    case queue_reason::FORECAST: os << "forecast"; break;
    case queue_reason::STANDING: os << "standing"; break;
    case queue_reason::TRAIN: os << "train"; break;
    case queue_reason::WAITING: os << "waiting"; break;
    case queue_reason::CANCELED: os << "canceled"; break;
    case queue_reason::RECALC: os << "recalc"; break;
    case queue_reason::REPAIR: os << "repair"; break;
  }
  return os;
}

}  // namespace realtime
}  // namespace motis
