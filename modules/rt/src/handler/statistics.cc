#include "motis/rt/handler/statistics.h"

namespace motis {
namespace rt {
namespace handler {

std::ostream& operator<<(std::ostream& out, trip_counter const& c) {
  out << " trips found: " << c.found << "/" << c.total << "\n";

  if ((c.missed_primary - c.fuzzy_primary - c.is_additional -
       c.primary_not_in_schedule) == 0) {
    out << "  [okay] ";
  } else {
    out << "  [MISS] ";
  }
  out << "missed primary: " << c.missed_primary
      << ", found fuzzy: " << c.fuzzy_primary
      << " (is_additional: " << c.is_additional
      << ", not in schedule: " << c.primary_not_in_schedule << ")\n";

  if ((c.missed_secondary - c.secondary_not_in_schedule) == 0) {
    out << "  [okay] ";
  } else {
    out << "  [MISS] ";
  }
  out << "missed secondary: " << c.missed_secondary << " (not in schedule "
      << c.secondary_not_in_schedule << ")\n";
  return out;
}

std::ostream& operator<<(std::ostream& out, message_counter const& c) {
  out << c.title_ << ": " << c.count_ << "\n";
  if (c.trips_.total != c.trips_.found) {
    out << c.trips_;
  }
  return out;
}

std::ostream& operator<<(std::ostream& out, statistics const& s) {
  out << "rt handler statistics:\n" << s.delay << s.additional << s.canceled
      << s.reroute << s.assessment << s.decision << "\n";
  return out;
}

}  // namespace handler
}  // namespace rt
}  // namespace motis
