#pragma once

#include "motis/module/module.h"
#include "motis/ris/mode/base_mode.h"

namespace motis {
namespace ris {
namespace mode {

struct test_mode : public base_mode {
  test_mode(config* conf) : base_mode(conf) {}

  virtual void init_async() override;
};

}  // namespace mode
}  // namespace ris
}  // namespace motis
