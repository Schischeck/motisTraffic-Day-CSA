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
  additional_train_message(std::string category,
                           std::vector<schedule_event> events)
      : message(message_type::additional),
        category_(category),
        events_(events) {}

  uint32_t train_nr_;
  std::string category_;
  std::vector<schedule_event> events_;
};

class cancel_train_message : public message {
public:
  cancel_train_message() : message(message_type::cancel) {}
  cancel_train_message(std::vector<schedule_event> events)
      : message(message_type::cancel), events_(events) {}

  uint32_t train_nr_;
  std::vector<schedule_event> events_;
};

class reroute_train_message : public message {
public:
  reroute_train_message() : message(message_type::reroute) {}
  reroute_train_message(std::string category,
                        std::vector<schedule_event> canceled_events,
                        std::vector<schedule_event> new_events)
      : message(message_type::reroute),
        category_(category),
        canceled_events_(canceled_events),
        new_events_(new_events) {}

  uint32_t train_nr_;
  std::string category_;
  std::vector<schedule_event> canceled_events_;
  std::vector<schedule_event> new_events_;
};

class connection_message : public message {
public:
  connection_message(message_type type) : message(type) {}
  connection_message(message_type type, const schedule_event& arrival,
                     const schedule_event& departure)
      : message(type), arrival_(arrival), departure_(departure) {}

  schedule_event arrival_;
  schedule_event departure_;
};

enum class status_decision { kept, unkept, new_connection, unknown };

class connection_status_decision_message : public connection_message {
public:
  connection_status_decision_message()
      : connection_message(message_type::status_decision) {}
  connection_status_decision_message(const schedule_event& arrival,
                                     const schedule_event& departure,
                                     status_decision decision)
      : connection_message(message_type::status_decision, arrival, departure),
        decision_(decision) {}

  status_decision decision_;
};

class connection_status_assessment_message : public connection_message {
public:
  connection_status_assessment_message()
      : connection_message(message_type::status_assessment) {}
  connection_status_assessment_message(const schedule_event& arrival,
                                       const schedule_event& departure,
                                       int assessment)
      : connection_message(message_type::status_assessment, arrival, departure),
        assessment_(assessment) {}

  int assessment_;
};

}  // namespace realtime
}  // namespace motis
