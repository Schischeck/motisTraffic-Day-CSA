#pragma once

#include <ctime>
#include <string>
#include <vector>

namespace motis {

struct journey {
  journey() = default;

  struct transport {
    unsigned from, to;
    enum transport_type { PublicTransport, Walk, Mumo } type;
    std::string name;
    std::string category_name;
    unsigned category_id;
    unsigned clasz;
    unsigned train_nr;
    std::string line_identifier;
    unsigned duration;
    int slot;
    std::string direction;
    std::string provider;
    unsigned route_id;

    std::string mumo_type_name;
    unsigned mumo_price;
  };

  struct stop {
    unsigned index;
    bool interchange;
    std::string name;
    std::string eva_no;
    double lat, lng;
    struct event_info {
      bool valid;
      std::time_t timestamp;
      std::time_t schedule_timestamp;
      std::string platform;
    } arrival, departure;
  };

  struct attribute {
    unsigned from, to;
    std::string code;
    std::string text;
  };

  uint16_t get_duration() const;
  uint16_t get_transfers() const;

  unsigned duration, transfers, price;
  std::vector<stop> stops;
  std::vector<transport> transports;
  std::vector<attribute> attributes;
};

}  // namespace motis
