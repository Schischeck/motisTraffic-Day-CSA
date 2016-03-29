#pragma once

#include <map>

#include "module/container.h"

namespace motis {
namespace module {

struct registry {
  std::map<std::string, operation> operations_;
  snapshot containers_;
};

}  // namespace module
}  // namespace motis
