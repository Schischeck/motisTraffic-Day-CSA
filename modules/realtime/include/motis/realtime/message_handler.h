#pragma once

#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "motis/core/schedule/delay_info.h"
#include "motis/core/schedule/event.h"
#include "motis/realtime/messages.h"

namespace motis {
namespace realtime {

class realtime_schedule;

class message_handler {
public:
  message_handler(realtime_schedule& rts);

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
};

}  // namespace realtime
}  // namespace motis
