#pragma once

#include <ctime>
#include <string>
#include <vector>

namespace motis {

struct journey {
  journey() = default;

  struct transport {
    unsigned int from, to;
    enum transport_type { PublicTransport, Walk, Mumo } type;

    std::string name;
    std::string category_name;
    unsigned int category_id;
    unsigned int train_nr;
    std::string line_identifier;
    unsigned int duration;
    int slot;
    std::string direction;
    std::string provider;
    unsigned int route_id;

    std::string mumo_type_name;
    unsigned short mumo_price;
  };

  struct stop {
    unsigned int index;
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
    unsigned int from, to;
    std::string code;
    std::string text;
  };

  uint16_t get_duration() const;
  uint16_t get_transfers() const;

  unsigned int duration, transfers, price, night_penalty;
  std::vector<stop> stops;
  std::vector<transport> transports;
  std::vector<attribute> attributes;
};

}  // namespace motis
