#pragma once

#include <vector>
#include <string>
#include <utility>
#include <unordered_map>

#include "motis/realtime/message_stream.h"
#include "motis/realtime/event.h"
#include "motis/realtime/delay_info.h"

namespace motis {
namespace realtime {

class realtime_schedule;

class message_handler {
public:
  message_handler(realtime_schedule& rts);

  void process_message_stream(message_stream& stream);

  void handle_message(const message& msg);

  void handle_delay(const schedule_event& schedule_event, motis::time new_time,
                    timestamp_reason reason);
  void handle_delay(const delay_message& msg);
  void handle_additional_train(const additional_train_message& msg);
  void handle_canceled_train(const cancel_train_message& msg);
  void handle_rerouted_train(const reroute_train_message& msg);
  void handle_connection_status_decision(
      const connection_status_decision_message& msg);

private:
  std::pair<uint32_t, uint8_t> get_or_create_category(std::string category);
  bool is_valid_train(const std::vector<schedule_event>& events) const;
  bool train_exists(const std::vector<schedule_event>& events) const;
  bool event_exists(const schedule_event& se) const;

  realtime_schedule& _rts;
  message_reader _msg_reader;
};

}  // namespace realtime
}  // namespace motis
