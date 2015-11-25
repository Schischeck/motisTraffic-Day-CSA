#include <motis/realtime/statistics.h>
#include "motis/realtime/delay_info.h"
#include "motis/realtime/realtime_schedule.h"
#include "motis/core/common/logging.h"

namespace motis {
namespace realtime {

using namespace motis::logging;

delay_info_manager::~delay_info_manager() {
  for (delay_info* delay_info : _delay_infos) {
    delete delay_info;
  }
  for (delay_info* delay_info : _buffered_delay_infos) {
    delete delay_info;
  }
}

delay_info* delay_info_manager::get_delay_info(
    const schedule_event& event_id) const {
  auto it = _schedule_map.find(event_id);
  if (it != _schedule_map.end()) {
    return it->second;
  } else {
    return nullptr;
  }
}

delay_info* delay_info_manager::get_buffered_delay_info(
    const schedule_event& event_id) const {
  auto it = _buffered_map.find(event_id);
  if (it != _buffered_map.end()) {
    return it->second;
  } else {
    return nullptr;
  }
}

delay_info* delay_info_manager::create_delay_info(
    const schedule_event& event_id, int32_t route_id) {
  assert(event_id.found());

  delay_info* di = get_buffered_delay_info(event_id);
  if (di == nullptr) {
    di = new delay_info(event_id);
    di->_route_id = route_id;
    assert(route_id != -1);

    _delay_infos.push_back(di);
    _schedule_map[event_id] = di;

    // no delay so far
    _current_map[di->graph_ev()] = di;
  } else {
    upgrade_delay_info(di, route_id);
  }

  return di;
}

delay_info* delay_info_manager::create_buffered_delay_info(
    const schedule_event& event_id) {
  assert(event_id.found());
  auto di = new delay_info(event_id);
  di->_route_id = -1;

  if (_schedule_map.find(event_id) != _schedule_map.end()) {
    LOG(warn) << "buffered delay info conflict: " << event_id;
  }

  _rts._stats._ops.delay_infos.buffered++;

  _buffered_map[event_id] = di;
  _buffered_delay_infos.push_back(di);

  return di;
}

void delay_info_manager::upgrade_delay_info(delay_info* di, int32_t route_id) {
  assert(di->_route_id == -1);
  assert(route_id != -1);

  _rts._stats._ops.delay_infos.upgrades++;

  _buffered_map.erase(di->sched_ev());
  _buffered_delay_infos.erase(std::remove(_buffered_delay_infos.begin(),
                                          _buffered_delay_infos.end(), di),
                              _buffered_delay_infos.end());

  di->_route_id = route_id;
  _delay_infos.push_back(di);
  _schedule_map[di->sched_ev()] = di;
  _current_map[di->graph_ev()] = di;
}

void delay_info_manager::update_delay_info(const delay_info_update* update) {
  delay_info* delay_info = update->_delay_info;

  if (_rts.is_tracked(delay_info->sched_ev()._train_nr)) {
    LOG(debug) << "updating delay info: " << *update;
  }

  // remove old entry from mapping
  auto it = _current_map.find(delay_info->graph_ev());
  if (it != _current_map.end()) {
    if (it->second == update->_delay_info) {
      _current_map.erase(it);
    } else {
      LOG(debug) << "update delay info: " << *update
                 << " - not removing old entry: " << *it->second;
    }
  } else {
    LOG(warn) << "old entry in current_map not found: " << *update;
  }

  delay_info->_current_time = update->_new_time;
  delay_info->_reason = update->_new_reason;

  assert(delay_info->_route_id != -1);
  // add new entry to mapping
  _current_map[delay_info->graph_ev()] = delay_info;
}

motis::time delay_info_manager::reset_to_schedule(
    const schedule_event& event_id) {
  delay_info* di = get_delay_info(event_id);
  if (di != nullptr) {
    if (di->_reason == timestamp_reason::IS ||
        di->_reason == timestamp_reason::REPAIR) {
      // make sure that we don't forget is messages
      _rts._delay_propagator.handle_delay_message(event_id, di->_current_time,
                                                  di->_reason);
    }
    delay_info_update update(di, event_id._schedule_time,
                             timestamp_reason::SCHEDULE);
    update_delay_info(&update);
  }
  return event_id._schedule_time;
}

void delay_info_manager::update_route(delay_info* di, int32_t new_route) {
  if (di->_route_id == new_route) {
    return;
  }
  // remove old entry from mapping
  auto it = _current_map.find(di->graph_ev());
  if (it != _current_map.end()) {
    assert(it->second == di);
    _current_map.erase(it);
  } else {
    LOG(warn) << "old entry in current_map not found: " << *di;
  }

  assert(new_route != -1);

  _rts._delay_propagator.update_route(di, new_route);

  di->_route_id = new_route;

  // add new entry to mapping
  _current_map[di->graph_ev()] = di;
}

delay_info* delay_info_manager::get_delay_info(
    const graph_event& event_id) const {
  auto it = _current_map.find(event_id);
  if (it != _current_map.end()) {
    return it->second;
  } else {
    return nullptr;
  }
}

delay_info* delay_info_manager::cancel_event(const schedule_event& event_id,
                                             int32_t route_id) {
  delay_info* di = get_delay_info(event_id);
  assert(route_id != -1);
  if (di == nullptr) {
    di = create_delay_info(event_id, route_id);
  } else {
    update_route(di, route_id);
  }
  di->_canceled = true;
  return di;
}

delay_info* delay_info_manager::undo_cancelation(
    const schedule_event& event_id) {
  delay_info* di = get_delay_info(event_id);
  if (di != nullptr) {
    di->_canceled = false;
  }
  return di;
}

motis::time delay_info_manager::current_time(
    const schedule_event& event_id) const {
  delay_info* delay_info = get_delay_info(event_id);
  if (delay_info != nullptr) {
    return delay_info->_current_time;
  } else {
    return event_id._schedule_time;
  }
}

std::ostream& operator<<(std::ostream& os, const timestamp_reason& r) {
  switch (r) {
    case timestamp_reason::SCHEDULE: os << "s"; break;
    case timestamp_reason::IS: os << "i"; break;
    case timestamp_reason::FORECAST: os << "f"; break;
    case timestamp_reason::PROPAGATION: os << "p"; break;
    case timestamp_reason::REPAIR: os << "r"; break;
  }
  return os;
}

}  // namespace realtime
}  // namespace motis
