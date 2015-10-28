#pragma once

#include <vector>
#include <string>
#include <utility>
#include <istream>
#include <unordered_map>

#include "motis/realtime/message_classes.h"
#include "motis/realtime/event.h"
#include "motis/realtime/delay_info.h"

namespace motis {
namespace realtime {

class realtime_schedule;

class message_handler {
public:
  message_handler(realtime_schedule& rts);

  void process_message_stream(std::istream& stream, bool eva_numbers = true);

  void handle_message(const message_class& msg);

  void handle_delay(const schedule_event& schedule_event, motis::time new_time,
                    timestamp_reason reason);
  void handle_delay(const delayed_train_message& msg);

  void handle_additional_train(std::vector<schedule_event> events,
                               std::string category);
  void handle_additional_train(const additional_or_canceled_train_message& msg);

  void handle_canceled_train(std::vector<schedule_event> canceled_events);
  void handle_canceled_train(const additional_or_canceled_train_message& msg);

  void handle_rerouted_train(std::vector<schedule_event> canceled_events,
                             std::vector<schedule_event> new_events,
                             std::string category);
  void handle_rerouted_train(const reroute_train_message& msg);

  void handle_connection_status_decision(
      const schedule_event& arrival, const schedule_event& departure,
      connection_status::decision_type status);
  void handle_connection_status_decision(
      const connection_status_decision_message& msg);

  const motis::station* get_station_by_eva(std::string eva_nr) const;

private:
  std::pair<uint32_t, uint8_t> get_or_create_category(std::string category);
  bool is_valid_train(const std::vector<schedule_event>& events) const;
  bool train_exists(const std::vector<schedule_event>& events) const;

  schedule_event get_schedule_event(const schedule_stop* stop,
                                    int train_nr) const;

  realtime_schedule& _rts;
};

}  // namespace realtime
}  // namespace motis
