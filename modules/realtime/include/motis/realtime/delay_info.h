#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>
#include <ostream>
#include <memory>

#include "boost/functional/hash.hpp"
#include "boost/operators.hpp"

#include "motis/core/common/util.h"
#include "motis/core/schedule/nodes.h"
#include "motis/core/schedule/edges.h"
#include "motis/core/schedule/connection.h"
#include "motis/core/schedule/schedule.h"
#include "motis/realtime/event.h"
#include "motis/realtime/waiting_edges.h"

namespace motis {
namespace realtime {

enum class timestamp_reason : uint8_t {
  SCHEDULE,
  IS,
  FORECAST,
  PROPAGATION,
  REPAIR
};

std::ostream& operator<<(std::ostream& os, const timestamp_reason& r);

class delay_info {
public:
  delay_info(class schedule_event schedule_event, int32_t route_id = -1)
      : _schedule_event(schedule_event),
        _route_id(route_id),
        _forecast_time(motis::INVALID_TIME),
        _current_time(schedule_event._schedule_time),
        _canceled(false),
        _reason(timestamp_reason::SCHEDULE) {}

  inline bool delayed() const {
    return _current_time != _schedule_event._schedule_time;
  }
  inline bool canceled() const { return _canceled; }

  inline schedule_event sched_ev() const { return _schedule_event; }

  inline graph_event graph_ev() const {
    return motis::realtime::graph_event(
        _schedule_event._station_index, _schedule_event._train_nr,
        _schedule_event._departure, _current_time, _route_id);
  }

  bool operator==(const delay_info& other) const {
    return _schedule_event == other._schedule_event;
  }

  friend std::ostream& operator<<(std::ostream& os, const delay_info& di) {
    os << "<event=" << di._schedule_event << ", route_id=" << di._route_id
       << ", fc=" << motis::format_time(di._forecast_time)
       << ", ct=" << motis::format_time(di._current_time)
       << ", reason=" << di._reason << (di.canceled() ? ", CANCELED" : "")
       << ">";
    return os;
  }

  class schedule_event _schedule_event;
  int32_t _route_id;

  motis::time _forecast_time;
  motis::time _current_time;
  bool _canceled;
  timestamp_reason _reason;
};

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
  delay_info_manager(realtime_schedule& rts) : _rts(rts) {}

  std::vector<std::unique_ptr<delay_info>> const& delay_infos() const { return _delay_infos; }
  delay_info* get_delay_info(const schedule_event& event_id) const;
  delay_info* get_buffered_delay_info(const schedule_event& event_id) const;
  delay_info* create_delay_info(const schedule_event& event_id,
                                int32_t route_id);
  delay_info* create_buffered_delay_info(const schedule_event& event_id);
  void update_delay_info(const delay_info_update* update);
  motis::time reset_to_schedule(const schedule_event& event_id);
  void update_route(delay_info* di, int32_t new_route);

  delay_info* get_delay_info(const graph_event& event_id) const;

  delay_info* cancel_event(const schedule_event& event_id, int32_t route_id);
  delay_info* undo_cancelation(const schedule_event& event_id);

  motis::time current_time(const schedule_event& event_id) const;

  void upgrade_delay_info(delay_info* di, int32_t route_id);

  std::vector<delay_info*> get_delay_info_delta();

private:
  realtime_schedule& _rts;
  std::vector<std::unique_ptr<delay_info>> _delay_infos;
  std::vector<std::unique_ptr<delay_info>> _buffered_delay_infos;

  std::vector<delay_info*> _updated_delay_infos;

  using schedule_event_hash = boost::hash<schedule_event>;
  using graph_event_hash = boost::hash<graph_event>;

  std::unordered_map<schedule_event, delay_info*, schedule_event_hash>
      _schedule_map;
  std::unordered_map<graph_event, delay_info*, graph_event_hash> _current_map;

  std::unordered_map<schedule_event, delay_info*, schedule_event_hash>
      _buffered_map;
};

}  // namespace realtime
}  // namespace motis
