#include <motis/realtime/statistics.h>
#include "motis/realtime/delay_info.h"
#include "motis/realtime/realtime_schedule.h"
#include "motis/core/common/logging.h"

namespace motis {
namespace realtime {

using namespace motis::logging;

delay_info* delay_info_manager::get_delay_info(schedule_event const& e) const {
  auto it = _schedule_map.find(e);
  return (it != end(_schedule_map)) ? it->second : nullptr;
}

delay_info* delay_info_manager::get_buffered_delay_info(
    schedule_event const& e) const {
  auto it = _buffered_map.find(e);
  return (it != _buffered_map.end()) ? it->second : nullptr;
}

delay_info* delay_info_manager::get_delay_info(graph_event const& e) const {
  auto it = _current_map.find(e);
  return (it != _current_map.end()) ? it->second : nullptr;
}

motis::time delay_info_manager::current_time(const schedule_event& e) const {
  delay_info* di = get_delay_info(e);
  return (di != nullptr) ? di->_current_time : e._schedule_time;
}

delay_info* delay_info_manager::create_delay_info(
    schedule_event const& event_id, int32_t route_id) {
  assert(event_id.found());

  auto buffered_di = get_buffered_delay_info(event_id);
  if (buffered_di != nullptr) {
    upgrade_delay_info(buffered_di, route_id);
    return buffered_di;
  }

  assert(route_id != -1);
  _delay_infos.push_back(make_unique<delay_info>(event_id, route_id));
  auto di = _delay_infos.back().get();

  _schedule_map[event_id] = di;
  _current_map[di->graph_ev()] = di;  // no delay so far

  _updated_delay_infos.push_back(di);
  return di;
}

delay_info* delay_info_manager::create_buffered_delay_info(
    const schedule_event& event_id) {
  assert(event_id.found());

  _buffered_delay_infos.push_back(make_unique<delay_info>(event_id, -1));
  auto di = _buffered_delay_infos.back().get();

  _buffered_map[event_id] = di;
  if (_schedule_map.find(event_id) != end(_schedule_map)) {

    LOG(warn) << "buffered delay info conflict: " << event_id;
  }
  _rts._stats._ops.delay_infos.buffered++;

  return di;
}

void delay_info_manager::upgrade_delay_info(delay_info* di, int32_t route_id) {
  assert(di->_route_id == -1);
  assert(route_id != -1);

  _rts._stats._ops.delay_infos.upgrades++;

  if (_rts.is_debug_mode()) {
    LOG(info) << "upgrading buffered di: " << *di << ", route_id=" << route_id;
  }

  auto it = std::find_if(
      begin(_buffered_delay_infos), end(_buffered_delay_infos),
      [&di](std::unique_ptr<delay_info> const& e) { return e.get() == di; });

  if (it == end(_buffered_delay_infos)) {
    throw std::runtime_error("could not find buffered_delay info.");
  }

  _delay_infos.emplace_back(std::move(*it));
  _buffered_delay_infos.erase(it);

  _buffered_map.erase(di->sched_ev());
  _current_map[di->graph_ev()] = di;
  _schedule_map[di->sched_ev()] = di;

  di->_route_id = route_id;
  auto is_time = INVALID_TIME;
  if (di->_reason == timestamp_reason::IS) {
    is_time = di->_current_time;
    di->_current_time = di->sched_ev()._schedule_time;
    di->_reason = timestamp_reason::SCHEDULE;
  }

  if (is_time != INVALID_TIME) {
    _rts._delay_propagator.handle_delay_message(di->sched_ev(), is_time,
                                                timestamp_reason::IS);
  }
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
  _updated_delay_infos.push_back(delay_info);
}

motis::time delay_info_manager::reset_to_schedule(
    const schedule_event& event_id) {
  auto const& di = get_delay_info(event_id);
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
  _updated_delay_infos.push_back(di);
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
  _updated_delay_infos.push_back(di);
  return di;
}

delay_info* delay_info_manager::undo_cancelation(
    const schedule_event& event_id) {
  delay_info* di = get_delay_info(event_id);
  if (di != nullptr) {
    di->_canceled = false;
    _updated_delay_infos.push_back(di);
  }
  return di;
}

std::vector<delay_info*> delay_info_manager::get_delay_info_delta() {
  std::vector<delay_info*> delta;
  std::swap(delta, _updated_delay_infos);  // retrieve delta and reset state

  std::sort(begin(delta), end(delta));
  delta.erase(std::unique(begin(delta), end(delta)), end(delta));
  return delta;
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
