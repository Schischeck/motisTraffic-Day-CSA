#pragma once

#include <set>
#include <vector>
#include <functional>
#include <map>
#include <unordered_map>

#include "boost/operators.hpp"

#include "motis/core/schedule/delay_info.h"
#include "motis/realtime/delay_info_manager.h"

namespace motis {
namespace realtime {

class realtime_schedule;

enum class queue_reason : uint8_t {
  IS,  // is message
  FORECAST,  // forecast message
  STANDING,  // standing train edge
  TRAIN,  // traveling train edge
  WAITING,  // waiting edge
  CANCELED,  // event canceled
  RECALC,  // recalculate (event added, cancelation revoked, other train
  // event canceled)
  REPAIR  // fake message to repair an otherwise broken train because of
  // conflicting is messages
};

std::ostream& operator<<(std::ostream& os, const queue_reason& r);

class delay_queue_entry
    : private boost::less_than_comparable<delay_queue_entry> {
public:
  delay_queue_entry(delay_info* di, queue_reason queue_reason)
      : _delay_info(di), _queue_reason(queue_reason) {}

  bool operator<(const delay_queue_entry& rhs) const {
    const schedule_event& se = _delay_info->sched_ev();
    const schedule_event& ose = rhs._delay_info->sched_ev();
    if (se._schedule_time == ose._schedule_time) {
      // in this case, we don't care about the order, but an order
      // must be defined, otherwise set thinks the entries are equal
      return _delay_info < rhs._delay_info;
    } else {
      return se._schedule_time < ose._schedule_time;
    }
  }

  bool operator==(const delay_queue_entry& rhs) const {
    return _delay_info == rhs._delay_info && _queue_reason == rhs._queue_reason;
  }

  delay_info* _delay_info;
  queue_reason _queue_reason;
};

struct delay_info_ptr_compare {
  bool operator()(const delay_info* lhs, const delay_info* rhs) const {
    return lhs->_schedule_event < rhs->_schedule_event;
  }
};

class delay_propagator {
public:
  delay_propagator(realtime_schedule& rts) : _rts(rts) {}

  void handle_delay_message(const schedule_event& event_id,
                            motis::time new_time, timestamp_reason reason);

  void process_queue();

  void enqueue(const schedule_event& event_id, queue_reason queue_reason,
               int32_t route_id = -1);
  void enqueue(delay_info* di, queue_reason reason);

  void update_route(delay_info* di, int32_t new_route);

private:
  bool calculate_max(const delay_queue_entry& entry);
  void queue_dependent_events(const delay_queue_entry& entry);
  void add_update(delay_info* di, motis::time new_time,
                  timestamp_reason new_reason);
  void apply_updates();

  motis::time new_time(delay_info* di) const;
  motis::time new_time(const schedule_event& event_id) const;
  timestamp_reason new_reason(delay_info* di) const;

  realtime_schedule& _rts;
  std::set<delay_queue_entry> _queue;
  // TODO: ordered map?
  // std::unordered_map<delay_info*, delay_info_update> _delay_info_updates;
  // std::map<delay_info*, delay_info_update, delay_info_ptr_compare>
  // _delay_info_updates;
  std::unordered_map<int32_t, std::vector<delay_info_update>>
      _delay_info_updates;
};

}  // namespace realtime
}  // namespace motis
