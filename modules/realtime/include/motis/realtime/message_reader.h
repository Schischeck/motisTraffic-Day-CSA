#pragma once

#include <memory>
#include <iostream>

#include "motis/realtime/messages.h"

namespace motis {
namespace realtime {
class realtime_schedule;

class message_reader {
public:
  message_reader(realtime_schedule& rts);

  std::unique_ptr<message> read_message(std::istream& in);

private:
  std::unique_ptr<delay_message> read_delay_message(std::istream& in);
  std::unique_ptr<additional_train_message> read_additional_train_message(
      std::istream& in);
  std::unique_ptr<cancel_train_message> read_cancel_train_message(
      std::istream& in);
  std::unique_ptr<reroute_train_message> read_reroute_train_message(
      std::istream& in);
  std::unique_ptr<connection_status_decision_message>
  read_connection_status_decision_message(std::istream& in);
  std::unique_ptr<connection_status_assessment_message>
  read_connection_status_assessment_message(std::istream& in);

  int eva_to_station_index(unsigned eva) const;
  motis::time to_time(std::time_t unix_ts) const;

  realtime_schedule& rts_;
};

}  // namespace realtime
}  // namespace motis
