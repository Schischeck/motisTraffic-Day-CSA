#pragma once

#include <string>
#include <vector>

#include "motis/path/db/kv_database.h"

namespace motis {
namespace path {

struct stat_result {
  enum class category {
    BETWEEN_STATIONS,
    IN_STATIONS,
    INVALID_ANGLES,
    UNKNOWN
  };

  stat_result(int clasz, category c, std::vector<std::string> station_sequences)
      : clasz_(clasz), category_(c), station_sequences_(station_sequences){};

  std::string category_str() const {
    switch (category_) {
      case category::IN_STATIONS: return "IN_STATIONS";
      case category::INVALID_ANGLES: return "INVALID_ANGLES";
      case category::BETWEEN_STATIONS: return "BETWEEN_STATIONS";
      case category::UNKNOWN: return "UNKNOWN";
      default: return "INVALID";
    }
  }

  int clasz_;
  category category_;
  std::vector<std::string> station_sequences_;
};

void dump_db_statistics(kv_database const&);

}  // namespace path
}  // namespace motis
