#pragma once

#include <ctime>
#include <string>
#include <vector>

#include "motis/core/schedule/delay_info.h"

namespace motis {

struct journey {
  struct transport {
    unsigned from_, to_;
    bool is_walk_;
    std::string name_;
    std::string category_name_;
    unsigned category_id_;
    unsigned clasz_;
    unsigned train_nr_;
    std::string line_identifier_;
    unsigned duration_;
    int mumo_id_;
    std::string direction_;
    std::string provider_;
    unsigned mumo_price_;
    std::string mumo_type_;
  };

  struct trip {
    unsigned from_, to_;

    std::string station_id_;
    uint32_t train_nr_;
    std::time_t time_;

    std::string target_station_id_;
    std::time_t target_time_;
    std::string line_id_;
  };

  struct stop {
    bool leave_, enter_;
    std::string name_;
    std::string eva_no_;
    double lat_, lng_;
    struct event_info {
      bool valid_;
      std::time_t timestamp_;
      std::time_t schedule_timestamp_;
      timestamp_reason timestamp_reason_;
      std::string track_;
    } arrival_, departure_;
  };

  struct attribute {
    unsigned from_, to_;
    std::string code_;
    std::string text_;
  };

  uint16_t get_duration() const;
  uint16_t get_transfers() const;

  unsigned duration_, transfers_, price_;
  std::vector<stop> stops_;
  std::vector<transport> transports_;
  std::vector<trip> trips_;
  std::vector<attribute> attributes_;

  unsigned night_penalty_, db_costs_;
};

}  // namespace motis
