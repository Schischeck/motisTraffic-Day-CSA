#pragma once

#include <ctime>
#include <string>
#include <vector>

#include "motis/core/schedule/delay_info.h"

namespace motis {

struct journey {
  struct transport {
    unsigned from_{0}, to_{0};
    bool is_walk_{false};
    std::string name_;
    std::string category_name_;
    unsigned category_id_{0};
    unsigned clasz_{0};
    unsigned train_nr_{0};
    std::string line_identifier_;
    unsigned duration_{0};
    int mumo_id_{0};
    std::string direction_;
    std::string provider_;
    unsigned mumo_price_{0};
    std::string mumo_type_;
  };

  struct trip {
    unsigned from_{0}, to_{0};

    std::string station_id_;
    uint32_t train_nr_{0};
    std::time_t time_{0};

    std::string target_station_id_;
    std::time_t target_time_{0};
    std::string line_id_;
  };

  struct stop {
    bool exit_{false}, enter_{false};
    std::string name_;
    std::string eva_no_;
    double lat_{0}, lng_{0};
    struct event_info {
      bool valid_{false};
      std::time_t timestamp_{0};
      std::time_t schedule_timestamp_{0};
      timestamp_reason timestamp_reason_{timestamp_reason::SCHEDULE};
      std::string track_;
    } arrival_, departure_;
  };

  struct attribute {
    unsigned from_{0}, to_{0};
    std::string code_;
    std::string text_;
  };

  unsigned duration_{0}, transfers_{0}, price_{0};
  std::vector<stop> stops_;
  std::vector<transport> transports_;
  std::vector<trip> trips_;
  std::vector<attribute> attributes_;

  unsigned night_penalty_{0}, db_costs_{0};
};

}  // namespace motis
