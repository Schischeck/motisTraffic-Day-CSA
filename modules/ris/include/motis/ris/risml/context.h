#pragma once

#include <limits>

#include "flatbuffers/flatbuffers.h"

namespace motis {
namespace ris {
namespace risml {

struct context {

  context()
      : earliest(std::numeric_limits<std::time_t>::max()),
        latest(std::numeric_limits<std::time_t>::min()) {}

  flatbuffers::FlatBufferBuilder b;
  std::time_t earliest, latest;
};

}  // namespace risml
}  // namespace ris
}  // namespace motis