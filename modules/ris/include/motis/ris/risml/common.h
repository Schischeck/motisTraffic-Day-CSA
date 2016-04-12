#pragma once

#include <ctime>
#include <limits>

#include "pugixml.hpp"

#include "flatbuffers/flatbuffers.h"

namespace motis {
namespace ris {
namespace risml {

struct context {

  context()
      : earliest(std::numeric_limits<std::time_t>::max()),
        latest(std::numeric_limits<std::time_t>::min()) {}

  flatbuffers::FlatBufferBuilder b_;
  std::time_t earliest_, latest_;
};

pugi::xml_attribute inline child_attr(pugi::xml_node const& n, char const* e,
                                      char const* a) {
  return n.child(e).attribute(a);
}

}  // namespace risml
}  // namespace ris
}  // namespace motis