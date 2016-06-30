#pragma once

#include <memory>

#include "motis/module/message.h"

namespace motis {
namespace osrm {

struct router {
  router(std::string path);
  ~router();

  motis::module::msg_ptr one_to_many(OSRMOneToManyRequest const*) const;
  motis::module::msg_ptr polyline(OSRMPolylineRequest const*) const;

  struct impl;
  std::unique_ptr<impl> impl_;
};

}  // namespace osrm
}  // namespace motis
