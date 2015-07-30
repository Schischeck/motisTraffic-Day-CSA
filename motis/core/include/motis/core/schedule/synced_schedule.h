#pragma once

#include "motis/core/schedule/schedule.h"

namespace motis {

enum schedule_access { RO, RW };

template <schedule_access A>
struct synced_schedule {};

template <>
struct synced_schedule<RO> {
  synced_schedule(schedule& s) : s_(s), lock_(s.sync, false) {}
  schedule const& sched() const { return s_; }
  schedule const& s_;
  synchronization::lock lock_;
};

template <>
struct synced_schedule<RW> {
  synced_schedule(schedule& s) : s_(s), lock_(s.sync, true) {}
  schedule& sched() { return s_; }
  schedule& s_;
  synchronization::lock lock_;
};

}  // namespace motis
