#pragma once

#include "motis/rt/handler/statistics.h"

namespace motis {
struct schedule;

namespace rt {
namespace handler {

struct context {
  schedule& sched;
  statistics stats;
};

}  // namespace handler
}  // namespace rt
}  // namespace motis
