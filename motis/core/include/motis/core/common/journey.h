#pragma once

#include <ctime>
#include <string>
#include <vector>

namespace motis {

struct journey {
  journey() = default;

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
    std::string direction;
    std::string provider;
  };

  struct stop {
    int index;
    bool interchange;
    std::string name;
    std::string eva_no;
    double lat, lng;
    struct event_info {
      bool valid;
      std::time_t timestamp;
      std::string platform;
    } arrival, departure;
  };

  struct attribute {
    int from, to;
    std::string code;
    std::string text;
  };

  int duration, transfers, price;
  std::vector<stop> stops;
  std::vector<transport> transports;
  std::vector<attribute> attributes;
};

}  // namespace motis
