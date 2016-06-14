#pragma once

#include <ostream>

namespace motis {

// The order is important:
// If (forecast time == schedule time), the reason should be FORECAST.
// Thus, FORECAST needs to appear after SCHEDULE (higher priority last).
enum class timestamp_reason { REPAIR, IS, SCHEDULE, PROPAGATION, FORECAST };

inline std::ostream& operator<<(std::ostream& out, timestamp_reason const& r) {
  switch (r) {
    case timestamp_reason::REPAIR: return out << "R";
    case timestamp_reason::SCHEDULE: return out << "S";
    case timestamp_reason::IS: return out << "I";
    case timestamp_reason::FORECAST: return out << "F";
    case timestamp_reason::PROPAGATION: return out << "P";
    default: return out;
  }
}

}  // namespace motis

// CONVERSION

#include "motis/protocol/TimestampReason_generated.h"

namespace motis {

inline TimestampReason to_fbs(timestamp_reason const r) {
  switch (r) {
    case timestamp_reason::REPAIR: return TimestampReason_REPAIR;
    case timestamp_reason::SCHEDULE: return TimestampReason_SCHEDULE;
    case timestamp_reason::IS: return TimestampReason_IS;
    case timestamp_reason::FORECAST: return TimestampReason_FORECAST;
    case timestamp_reason::PROPAGATION: return TimestampReason_PROPAGATION;
    default: return TimestampReason_SCHEDULE;
  }
}

inline timestamp_reason from_fbs(TimestampReason const r) {
  switch (r) {
    case TimestampReason_REPAIR: return timestamp_reason::REPAIR;
    case TimestampReason_SCHEDULE: return timestamp_reason::SCHEDULE;
    case TimestampReason_IS: return timestamp_reason::IS;
    case TimestampReason_FORECAST: return timestamp_reason::FORECAST;
    case TimestampReason_PROPAGATION: return timestamp_reason::PROPAGATION;
    default: return timestamp_reason::SCHEDULE;
  }
}

}  // namespace motis
