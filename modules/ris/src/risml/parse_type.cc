#include "motis/ris/risml/parse_type.h"

#include <map>

using namespace flatbuffers;
using namespace parser;

namespace motis {
namespace ris {
namespace risml {

boost::optional<EventType> parse_type(
    cstr const& raw, boost::optional<EventType> const default_value) {
  static const std::map<cstr, EventType> map({{"Start", EventType_Departure},
                                              {"Ab", EventType_Departure},
                                              {"An", EventType_Arrival},
                                              {"Ziel", EventType_Arrival}});
  auto it = map.find(raw);
  if (it == end(map)) {
    return default_value;
  }
  return it->second;
}

}  // risml
}  // namespace ris
}  // namespace motis
