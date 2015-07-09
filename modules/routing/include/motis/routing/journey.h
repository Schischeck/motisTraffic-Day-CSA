#pragma once

#include <string>
#include <vector>

#include "motis/routing/label.h"

namespace motis {

class label;
struct schedule;

struct journey {
  journey() = default;
  journey(label const* label, schedule const& sched);

  struct transport {
    int from, to;
    bool walk;
    std::string name;
    std::string category_name;
    int category_id;
    int train_nr;
    std::string line_identifier;
    int duration;
    int slot;
  };

  struct stop {
    int index;
    bool interchange;
    std::string name;
    int eva_no;
    double lat, lng;
    struct event_info {
      bool valid;
      std::string date_time;
      std::string platform;
    } arrival, departure;
  };

  struct attribute {
    int from, to;
    std::string code;
    std::string text;
  };

  std::string date;
  int duration, transfers, price;
  std::vector<stop> stops;
  std::vector<transport> transports;
  std::vector<attribute> attributes;
};

}  // namespace motis
