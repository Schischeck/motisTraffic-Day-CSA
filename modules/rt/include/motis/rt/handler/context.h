#pragma once

namespace motis {
struct schedule;

namespace rt {
namespace handler {

struct statistics {
  unsigned long ds100 = 0;
  unsigned long found_trips = 0;

  unsigned long missed_primary = 0;
  unsigned long missed_secondary = 0;
};

struct context {
  schedule const& sched;
  statistics stats;
};

}  // namespace handler
}  // namespace rt
}  // namespace motis
