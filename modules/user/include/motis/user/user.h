#pragma once

#include "motis/module/module.h"

namespace motis {
namespace user {

struct user : public motis::module::module {
  user();
  ~user() override = default;

  std::string name() const override { return "user"; }
  void init(motis::module::registry&) override;

private:
  motis::module::msg_ptr register_user(motis::module::msg_ptr const&);

  std::string conninfo_;
};

}  // namespace user
}  // namespace motis
