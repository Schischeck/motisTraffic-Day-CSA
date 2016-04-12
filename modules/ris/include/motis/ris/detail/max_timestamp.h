#pragma once

#include <ctime>
#include <algorithm>
#include <vector>

#include "motis/ris/blob.h"
#include "motis/ris/ris_message.h"

namespace motis {
namespace ris {
namespace detail {

inline std::time_t max_timestamp(std::vector<ris_message> const& msgs) {
  if (msgs.empty()) {
    return 0;
  }
  return std::max_element(begin(msgs), end(msgs),
                          [](ris_message const& lhs, ris_message const& rhs) {
                            return lhs.timestamp_ < rhs.timestamp_;
                          })
      ->timestamp_;
}

inline std::time_t max_timestamp(
    std::vector<std::pair<std::time_t, blob>> const& msgs) {
  if (msgs.empty()) {
    return 0;
  }
  return std::max_element(begin(msgs), end(msgs),
                          [](std::pair<std::time_t, blob> const& lhs,
                             std::pair<std::time_t, blob> const& rhs) {
                            return lhs.first < rhs.first;
                          })
      ->first;
}

}  // namespace detail
}  // namespace ris
}  // namespace motis
