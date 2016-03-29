#pragma once

#include "motis/module/dispatcher.h"
#include "motis/module/container.h"

namespace motis {
namespace module {

struct ctx_data {
  dispatcher* dispatcher_;
  std::shared_ptr<snapshot> snapshot_;
};

}  // namespace module
}  // namespace motis
