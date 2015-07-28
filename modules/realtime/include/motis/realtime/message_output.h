#pragma once

#include <ctime>
#include <vector>
#include <iostream>

#include "motis/realtime/message_classes.h"
#include "motis/realtime/delay_info.h"

namespace motis {
namespace realtime {

class realtime_schedule;

class message_output {
public:
  message_output(realtime_schedule& rts);

  void add_delay(delay_info const* di);
  void add_message(message_class const* msg);
  void set_current_time(std::time_t t);
  void finish();

private:
  void write_stream(std::ostream& out);

  std::time_t unix_time(time td_time) const {
    return _base_time + (td_time % MINUTES_A_DAY) * 60;
  }

  int reason_number(timestamp_reason reason) {
    switch (reason) {
      case timestamp_reason::SCHEDULE:
        return 0;
      case timestamp_reason::FORECAST:
        return 1;
      case timestamp_reason::IS:
        return 2;
      case timestamp_reason::PROPAGATION:
        return 3;
      default:
        return 0;
    }
  }

  realtime_schedule& _rts;
  std::time_t _base_time;
  std::time_t _current_time;
  std::vector<delay_info const*> _delays;
  std::vector<message_class const*> _messages;
};

}  // namespace realtime
}  // namespace motis
