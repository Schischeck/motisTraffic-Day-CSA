#pragma once

#include <vector>
#include <utility>
#include <unordered_map>

#include "motis/realtime/event.h"
#include "motis/core/schedule/connection.h"
#include "motis/core/schedule/nodes.h"

namespace motis {
namespace realtime {

// TODO: maybe store stops instead (dep+arrival event at same station)
class original_train_event {
public:
  original_train_event(const schedule_event& schedule_event,
                       const connection* connection, bool entering_allowed,
                       bool leaving_allowed)
      : _schedule_event(schedule_event),
        _connection(connection),
        _entering_allowed(entering_allowed),
        _leaving_allowed(leaving_allowed) {}

  schedule_event _schedule_event;
  const connection* _connection;
  bool _entering_allowed;
  bool _leaving_allowed;
};

class realtime_schedule;

class modified_train {
public:
  modified_train(int32_t original_route_id, int32_t new_route_id,
                 connection_info* connection_info, uint8_t clasz)
      : _original_route_id(original_route_id),
        _new_route_id(new_route_id),
        _connection_info(connection_info),
        _clasz(clasz) {}

  const original_train_event* original_event(
      const schedule_event& event_id) const;
  const original_train_event* original_event_at_station(
      unsigned station_index) const;

  int32_t _original_route_id;
  int32_t _new_route_id;
  std::vector<original_train_event> _original_events;
  schedule_event _current_start_event;
  connection_info* _connection_info;
  uint8_t _clasz;
};

class modified_train_manager {
public:
  void add(modified_train* mt);
  modified_train* train_with_event(const schedule_event& event_id) const;
  modified_train* train_with_route_id(int32_t route_id) const;

  std::unordered_map<int32_t, modified_train*> _modified_trains;
  std::unordered_map<schedule_event, modified_train*,
                     boost::hash<schedule_event>> _train_with_event;
};

}  // namespace realtime
}  // namespace motis
