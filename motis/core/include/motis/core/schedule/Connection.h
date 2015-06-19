#pragma once

#include <string>
#include <vector>
#include <tuple>

#include "motis/core/common/array.h"
#include "motis/core/common/pointer.h"
#include "motis/core/schedule/time.h"

namespace td {

enum {
  TD_ICE = 0,
  TD_IC = 1,
  TD_N = 2,
  TD_RE = 3,
  TD_RB = 4,
  TD_S = 5,
  TD_U = 6,
  TD_STR = 7,
  TD_BUS = 8,
  TD_X = 9
};

class connection_info {
public:
  connection_info()
      : attributes(array<int>::size_type(0)),
        line_identifier(string::size_type(0)),
        family(0),
        train_nr(0),
        service(0) {}

  bool operator<(connection_info const& o) const {
    return as_tuple() < o.as_tuple();
  }

  bool operator==(connection_info const& o) const {
    return attributes == o.attributes && line_identifier == o.line_identifier &&
           family == o.family && train_nr == o.train_nr && service == o.service;
  }

  std::tuple<array<int>, string, uint32_t, uint32_t, uint32_t> as_tuple()
      const {
    return std::make_tuple(attributes, line_identifier, family, train_nr,
                           service);
  }

  array<int> attributes;
  string line_identifier;
  uint32_t family;
  uint32_t train_nr;
  uint32_t service;
};

class connection {
public:
  connection()
      : con_info_id(0), price(0), d_platform(0), a_platform(0), clasz(0) {}

  bool operator==(connection const& o) const {
    return clasz == o.clasz && price == o.price && con_info_id == o.con_info_id;
  }

  union {
    pointer<connection_info const> con_info;
    uint32_t con_info_id;
  };
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

  pointer<connection const> _full_con;
  time d_time, a_time;

  enum : uint32_t { INVALID_CON_ID = 0xffffffff };
};

}  // namespace td
