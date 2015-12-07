#include "motis/realtime/modified_train.h"
#include "motis/realtime/realtime_schedule.h"

namespace motis {
namespace realtime {

void modified_train_manager::add(modified_train* mt) {
  _owner.emplace_back(mt);
  _modified_trains[mt->_new_route_id] = mt;
  for (const original_train_event& ote : mt->_original_events) {
    _train_with_event[ote._schedule_event] = mt;
  }
}

const original_train_event* modified_train::original_event(
    const schedule_event& event_id) const {
  for (const original_train_event& ote : _original_events) {
    if (ote._schedule_event == event_id) return &ote;
  }
  return nullptr;
}

const original_train_event* modified_train::original_event_at_station(
    unsigned station_index) const {
  for (const original_train_event& ote : _original_events) {
    if (ote._schedule_event._station_index == station_index) return &ote;
  }
  return nullptr;
}

modified_train* modified_train_manager::train_with_event(
    const schedule_event& event_id) const {
  auto it = _train_with_event.find(event_id);
  if (it != std::end(_train_with_event)) {
    return it->second;
  } else {
    return nullptr;
  }
}

modified_train* modified_train_manager::train_with_route_id(
    int32_t route_id) const {
  auto it = _modified_trains.find(route_id);
  if (it != std::end(_modified_trains)) {
    return it->second;
  } else {
    return nullptr;
  }
}

}  // namespace realtime
}  // namespace motis
