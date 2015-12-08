#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>
#include <ostream>
#include <memory>

#include "boost/functional/hash.hpp"
#include "boost/operators.hpp"

#include "motis/core/common/hash_map.h"

#include "motis/core/common/util.h"
#include "motis/core/schedule/connection.h"
#include "motis/core/schedule/edges.h"
#include "motis/core/schedule/event.h"
#include "motis/core/schedule/nodes.h"
#include "motis/core/schedule/schedule.h"
#include "motis/realtime/waiting_edges.h"

namespace motis {
namespace realtime {

class delay_info_update
    : private boost::less_than_comparable<delay_info_update> {
public:
  delay_info_update(delay_info* delay_info, motis::time new_time,
                    timestamp_reason new_reason)
      : _delay_info(delay_info), _new_time(new_time), _new_reason(new_reason) {}
  delay_info_update()
      : _delay_info(nullptr),
        _new_time(motis::INVALID_TIME),
        _new_reason(timestamp_reason::SCHEDULE) {}

  bool valid() const { return _delay_info != nullptr; }

  friend std::ostream& operator<<(std::ostream& os,
                                  const delay_info_update& diu) {
    if (diu.valid()) {
      os << "<di=" << *diu._delay_info
         << ", new_time=" << motis::format_time(diu._new_time)
         << ", new_reason=" << diu._new_reason << ">";
    } else {
      os << "<invalid>";
    }
    return os;
  }

  bool operator<(const delay_info_update& rhs) const {
    return _delay_info->_schedule_event < rhs._delay_info->_schedule_event;
  }

  delay_info* _delay_info;
  motis::time _new_time;
  timestamp_reason _new_reason;
};

class realtime_schedule;

class delay_info_manager {
public:
  delay_info_manager(realtime_schedule& rts) : _rts(rts) {
    constexpr auto inv_u = std::numeric_limits<uint32_t>::max();
    _buffered_map.set_empty_key({inv_u, inv_u, true, INVALID_TIME});
    _buffered_map.set_deleted_key({inv_u-1, inv_u, true, INVALID_TIME});
  }

  std::vector<std::unique_ptr<delay_info>> const& delay_infos() const;
  delay_info* get_delay_info(const schedule_event& event_id) const;
  delay_info* get_delay_info(const graph_event& event_id) const;
  
  delay_info* get_buffered_delay_info(const schedule_event& event_id) const;
  motis::time current_time(const schedule_event& event_id) const;

  delay_info* create_delay_info(const schedule_event& event_id,
                                int32_t route_id);
  delay_info* create_buffered_delay_info(const schedule_event& event_id);

  void update_delay_info(const delay_info_update* update);

  motis::time reset_to_schedule(const schedule_event& event_id);
  void update_route(delay_info* di, int32_t new_route);

  delay_info* cancel_event(const schedule_event& event_id, int32_t route_id);
  delay_info* undo_cancelation(const schedule_event& event_id);

  void upgrade_delay_info(delay_info* di, int32_t route_id);

  std::vector<delay_info*> get_delay_info_delta();

private:
  realtime_schedule& _rts;
  std::vector<std::unique_ptr<delay_info>> _buffered_delay_infos;
  hash_map<schedule_event, delay_info*> _buffered_map;

  std::vector<delay_info*> _updated_delay_infos;
};

}  // namespace realtime
}  // namespace motis
