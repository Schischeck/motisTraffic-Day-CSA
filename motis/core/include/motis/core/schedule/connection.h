#pragma once

#include <string>
#include <vector>
#include <tuple>

#include "motis/core/schedule/time.h"

namespace motis {

enum {
  MOTIS_ICE = 0,
  MOTIS_IC = 1,
  MOTIS_N = 2,
  MOTIS_RE = 3,
  MOTIS_RB = 4,
  MOTIS_S = 5,
  MOTIS_U = 6,
  MOTIS_STR = 7,
  MOTIS_BUS = 8,
  MOTIS_X = 9
};

class connection_info {
public:
  connection_info() : family(0), train_nr(0), service(0) {}

  bool operator<(connection_info const& o) const {
    return as_tuple() < o.as_tuple();
  }

  bool operator==(connection_info const& o) const {
    return attributes == o.attributes && line_identifier == o.line_identifier &&
           family == o.family && train_nr == o.train_nr && service == o.service;
  }

  std::tuple<std::vector<int>, std::string, uint32_t, uint32_t, uint32_t>
  as_tuple() const {
    return std::make_tuple(attributes, line_identifier, family, train_nr,
                           service);
  }

  std::vector<int> attributes;
  std::string line_identifier;
  uint32_t family;
  uint32_t train_nr;
  uint32_t service;
};

class connection {
public:
  connection()
      : con_info(nullptr), price(0), d_platform(0), a_platform(0), clasz(0) {}

  bool operator==(connection const& o) const {
    return clasz == o.clasz && price == o.price && con_info == o.con_info;
  }

  connection_info const* con_info;
  uint16_t price;
  uint16_t d_platform, a_platform;
  uint8_t clasz;
};

class light_connection {
public:
  light_connection() = default;

  explicit light_connection(time d_time) : d_time(d_time) {}

  light_connection(time d_time, time a_time, connection const* full_con)
      : _full_con(full_con), d_time(d_time), a_time(a_time) {}

  unsigned travel_time() const { return a_time - d_time; }

  bool operator<(light_connection const& o) const { return d_time < o.d_time; }

  bool operator==(light_connection const& o) const {
    return d_time == o.d_time && a_time == o.a_time &&
           *_full_con == *o._full_con;
  }

  connection const* _full_con;
  time d_time, a_time;

  enum : uint32_t { INVALID_CON_ID = 0xffffffff };
};

}  // namespace motis
