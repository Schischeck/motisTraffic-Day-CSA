#pragma once

namespace motis {
struct schedule;

namespace rt {
namespace handler {

struct statistics {
  unsigned long ds100 = 0;

  unsigned long trip_lookups = 0;
  unsigned long found_trips = 0;

  unsigned long missed_primary = 0;
  unsigned long no_train_nr_didnt_help = 0;
  unsigned long station_train_nr_miss = 0;

  unsigned long missed_secondary = 0;
};

struct context {
  schedule const& sched;
  statistics stats;
};

}  // namespace handler
}  // namespace rt
}  // namespace motis
