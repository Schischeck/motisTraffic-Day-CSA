#pragma once

#include <cinttypes>

namespace motis {

enum class event_type : uint8_t { DEP, ARR };

}  // namespace motis

// CONVERSION

#include "motis/protocol/EventType_generated.h"

namespace motis {

inline event_type from_fbs(EventType const ev_type) {
  return ev_type == EventType::EventType_DEP ? event_type::DEP
                                             : event_type::ARR;
}

inline EventType to_fbs(event_type const ev_type) {
  return ev_type == event_type::DEP ? EventType_DEP : EventType_ARR;
}

}  // namespace motis
