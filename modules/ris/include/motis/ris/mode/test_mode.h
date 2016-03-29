#pragma once

#include "motis/module/module.h"
#include "motis/ris/mode/base_mode.h"

namespace motis {
namespace ris {
namespace mode {

struct test_mode : public base_mode {
  test_mode(ris* module) : base_mode(module) {}

  virtual void init_async() override;
  virtual void on_msg(motis::module::msg_ptr, motis::module::sid,
                      motis::module::callback) override;
};

}  // namespace mode
}  // namespace ris
}  // namespace motis
