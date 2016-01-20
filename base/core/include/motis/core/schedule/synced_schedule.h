#pragma once

#include "motis/core/schedule/schedule.h"

namespace motis {

enum schedule_access { RO, RW, RE };

struct synced_schedule {
  synced_schedule(schedule& s, schedule_access access,
                  std::function<void()> on_destruct)
      : s_(s),
        on_destruct_(on_destruct),
        lock_(access != RE ? new synchronization::lock(s.sync, access == RW)
                           : nullptr) {}

  virtual ~synced_schedule() { on_destruct_(); }

  schedule& sched() { return s_; }

private:
  schedule& s_;
  std::function<void()> on_destruct_;
  std::unique_ptr<synchronization::lock> lock_;
};

}  // namespace motis
