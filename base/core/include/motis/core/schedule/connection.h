#pragma once

#include <string>
#include <vector>
#include <tuple>

#include "motis/core/common/hash_helper.h"
#include "motis/core/schedule/time.h"
#include "motis/core/schedule/attribute.h"
#include "motis/core/schedule/provider.h"
#include "motis/core/schedule/trip_idx.h"

namespace motis {

constexpr auto kMaxValidTrainNr = 99999;

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

struct connection_info {
  struct hash {
    std::size_t operator()(connection_info const& c) const {
      std::size_t seed = 0;
      for (auto const& attr : c.attributes) {
        hash_combine(seed, attr);
      }
      hash_combine(seed, c.line_identifier);
      hash_combine(seed, c.family);
      hash_combine(seed, c.train_nr);
      hash_combine(seed, c.merged_with);
      return seed;
    }
  };

  connection_info()
      : dir_(nullptr),
        provider_(nullptr),
        family(0),
        train_nr(0),
        original_train_nr(0),
        merged_with(nullptr) {}

  bool operator==(connection_info const& o) const {
    return train_nr == o.train_nr && family == o.family && dir_ == o.dir_ &&
           line_identifier == o.line_identifier && attributes == o.attributes &&
           merged_with == o.merged_with;
  }

  std::vector<attribute const*> attributes;
  std::string line_identifier;
  std::string const* dir_;
  provider const* provider_;
  uint32_t family;
  uint32_t train_nr;
  uint32_t original_train_nr;
  connection_info* merged_with;
};

struct connection {
  struct hash {
    std::size_t operator()(connection const& c) const {
      std::size_t seed = 0;
      hash_combine(seed, c.con_info);
      hash_combine(seed, c.price);
      hash_combine(seed, c.d_platform);
      hash_combine(seed, c.a_platform);
      hash_combine(seed, c.clasz);
      return seed;
    }
  };

  connection()
      : con_info(nullptr), price(0), d_platform(0), a_platform(0), clasz(0) {}

  bool operator==(connection const& o) const {
    return clasz == o.clasz && price == o.price && con_info == o.con_info;
  }

  bool operator<(connection const& o) const {
    return as_tuple() < o.as_tuple();
  }

  std::tuple<uint8_t, uint16_t, uint16_t, uint16_t, connection_info const*>
  as_tuple() const {
    return std::make_tuple(clasz, d_platform, a_platform, price, con_info);
  }

  connection_info const* con_info;
  uint16_t price;
  uint16_t d_platform, a_platform;
  uint8_t clasz;
};

struct light_connection {
  light_connection() = default;

  explicit light_connection(time d_time) : d_time(d_time) {}

  light_connection(time d_time, time a_time,
                   connection const* full_con = nullptr,
                   merged_trips_idx trips = 0)
      : _full_con(full_con), d_time(d_time), a_time(a_time), trips(trips) {}

  unsigned travel_time() const { return a_time - d_time; }

  bool operator<(light_connection const& o) const { return d_time < o.d_time; }

  bool operator==(light_connection const& o) const {
    return d_time == o.d_time && a_time == o.a_time &&
           *_full_con == *o._full_con;
  }

  connection const* _full_con;
  time d_time, a_time;
  merged_trips_idx trips;
};

}  // namespace motis
