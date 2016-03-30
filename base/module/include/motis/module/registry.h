#pragma once

#include <map>

#include "motis/module/container.h"
#include "motis/module/operation.h"

namespace motis {
namespace module {

struct registry {
  std::map<std::string, operation> operations_;
  snapshot containers_;
};

}  // namespace module
}  // namespace motis
