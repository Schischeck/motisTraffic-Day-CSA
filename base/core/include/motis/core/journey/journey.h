#pragma once

#include <ctime>
#include <string>
#include <vector>

namespace motis {

struct journey {
  journey() = default;

  struct transport {
    unsigned from_, to_;
    enum transport_type { PublicTransport, Walk, Mumo } type_;
    std::string name_;
    std::string category_name_;
    unsigned category_id_;
    unsigned clasz_;
    unsigned train_nr_;
    std::string line_identifier_;
    unsigned duration_;
    int slot_;
    std::string direction_;
    std::string provider_;

    std::string mumo_type_name_;
    unsigned mumo_price_;
  };

  struct stop {
    bool interchange_;
    std::string name_;
    std::string eva_no_;
    double lat_, lng_;
    struct event_info {
      bool valid_;
      std::time_t timestamp_;
      std::time_t schedule_timestamp_;
      std::string platform_;
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
  std::vector<attribute> attributes_;
};

}  // namespace motis
