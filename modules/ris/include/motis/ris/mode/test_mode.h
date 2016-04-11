#pragma once

#include "motis/module/module.h"
#include "motis/ris/mode/base_mode.h"

namespace motis {
namespace ris {
namespace mode {

struct test_mode final : public base_mode {
  test_mode(config* conf) : base_mode(conf) {}

  void init_async() override;
};

}  // namespace mode
}  // namespace ris
}  // namespace motis
