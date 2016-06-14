#pragma once

#include <string>
#include <tuple>
#include <vector>

#include "motis/core/common/hash_helper.h"
#include "motis/core/schedule/attribute.h"
#include "motis/core/schedule/provider.h"
#include "motis/core/schedule/time.h"
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
      for (auto const& attr : c.attributes_) {
        hash_combine(seed, attr);
      }
      hash_combine(seed, c.line_identifier_);
      hash_combine(seed, c.family_);
      hash_combine(seed, c.train_nr_);
      hash_combine(seed, c.merged_with_);
      return seed;
    }
  };

  connection_info()
      : dir_(nullptr),
        provider_(nullptr),
        family_(0),
        train_nr_(0),
        original_train_nr_(0),
        merged_with_(nullptr) {}

  bool operator==(connection_info const& o) const {
    return train_nr_ == o.train_nr_ && family_ == o.family_ && dir_ == o.dir_ &&
           line_identifier_ == o.line_identifier_ &&
           attributes_ == o.attributes_ && merged_with_ == o.merged_with_;
  }

  std::vector<attribute const*> attributes_;
  std::string line_identifier_;
  std::string const* dir_;
  provider const* provider_;
  uint32_t family_;
  uint32_t train_nr_;
  uint32_t original_train_nr_;
  connection_info* merged_with_;
};

struct connection {
  struct hash {
    std::size_t operator()(connection const& c) const {
      std::size_t seed = 0;
      hash_combine(seed, c.con_info_);
      hash_combine(seed, c.price_);
      hash_combine(seed, c.d_track_);
      hash_combine(seed, c.a_track_);
      hash_combine(seed, c.clasz_);
      return seed;
    }
  };

  connection()
      : con_info_(nullptr), price_(0), d_track_(0), a_track_(0), clasz_(0) {}

  bool operator==(connection const& o) const {
    return clasz_ == o.clasz_ && price_ == o.price_ && con_info_ == o.con_info_;
  }

  bool operator<(connection const& o) const {
    return as_tuple() < o.as_tuple();
  }

  std::tuple<uint8_t, uint16_t, uint16_t, uint16_t, connection_info const*>
  as_tuple() const {
    return std::make_tuple(clasz_, d_track_, a_track_, price_, con_info_);
  }

  connection_info const* con_info_;
  uint16_t price_;
  uint16_t d_track_, a_track_;
  uint8_t clasz_;
};

struct light_connection {
  light_connection() = default;

  explicit light_connection(time d_time) : d_time_(d_time) {}

  light_connection(time d_time, time a_time,
                   connection const* full_con = nullptr,
                   merged_trips_idx trips = 0)
      : full_con_(full_con),
        d_time_(d_time),
        a_time_(a_time),
        trips_(trips),
        valid_(true) {}

  unsigned travel_time() const { return a_time_ - d_time_; }

  bool operator<(light_connection const& o) const {
    return d_time_ < o.d_time_;
  }

  bool operator==(light_connection const& o) const {
    return d_time_ == o.d_time_ && a_time_ == o.a_time_ &&
           *full_con_ == *o.full_con_;
  }

  connection const* full_con_;
  time d_time_, a_time_;
  uint32_t trips_ : 31;
  uint32_t valid_ : 1;
};

}  // namespace motis
