#pragma once

#include "motis/module/module.h"

namespace motis {
namespace cc {

struct cc : public motis::module::module {
  cc() : module("Connection Checker", "cc") {}
  void init(motis::module::registry&) override;

private:
  motis::module::msg_ptr check_journey(motis::module::msg_ptr const&) const;
};

}  // namespace cc
}  // namespace motis
