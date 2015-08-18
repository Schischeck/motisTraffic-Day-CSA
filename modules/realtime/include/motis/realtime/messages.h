#pragma once

#include <cstdint>
#include <ctime>
#include <string>
#include <vector>

#include "boost/operators.hpp"

#include "motis/core/schedule/time.h"

#include "motis/realtime/event.h"

namespace motis {
namespace realtime {

enum class message_type {
  none,
  delay,
  additional,
  cancel,
  reroute,
  status_decision,
  status_assessment
};

class message : private boost::less_than_comparable<message> {
public:
  message() : type_(message_type::none), release_time_(0) {}
  message(message_type type) : type_(type), release_time_(0) {}

  bool operator<(const message& rhs) const {
    return release_time_ < rhs.release_time_;
  }

  message_type type_;
  std::time_t release_time_;
};

struct delayed_event : public schedule_event {
  delayed_event() : schedule_event(), delayed_time_(INVALID_TIME) {}
  delayed_event(const schedule_event& event, motis::time delayed_time)
      : schedule_event(event), delayed_time_(delayed_time) {}
  delayed_event(unsigned station_index, uint32_t train_nr, bool departure,
                motis::time schedule_time, motis::time delayed_time)
      : schedule_event(station_index, train_nr, departure, schedule_time),
        delayed_time_(delayed_time) {}

  motis::time delayed_time_;
};

class delay_message : public message {
public:
  delay_message() : message(message_type::delay) {}

  uint32_t train_nr_;
  delayed_event is_;
  std::vector<delayed_event> forecasts_;
};

class additional_train_message : public message {
public:
  additional_train_message() : message(message_type::additional) {}

  uint32_t train_nr_;
  std::string category_;
  std::vector<schedule_event> events_;
};

class cancel_train_message : public message {
public:
  cancel_train_message() : message(message_type::cancel) {}

  uint32_t train_nr_;
  std::vector<schedule_event> events_;
};

class reroute_train_message : public message {
public:
  reroute_train_message() : message(message_type::reroute) {}

  uint32_t train_nr_;
  std::string category_;
  std::vector<schedule_event> canceled_events_;
  std::vector<schedule_event> new_events_;
};

class connection_message : public message {
public:
  connection_message(message_type type) : message(type) {}

  schedule_event arrival_;
  schedule_event departure_;
};

enum class status_decision { kept, unkept, new_connection, unknown };

class connection_status_decision_message : public connection_message {
public:
  connection_status_decision_message()
      : connection_message(message_type::status_decision) {}

  status_decision decision_;
};

class connection_status_assessment_message : public connection_message {
public:
  connection_status_assessment_message()
      : connection_message(message_type::status_assessment) {}

  int assessment_;
};

}  // namespace realtime
}  // namespace motis
