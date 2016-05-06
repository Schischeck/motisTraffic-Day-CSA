#pragma once

#include <memory>
#include <vector>

#include "motis/module/module.h"

namespace motis {
namespace rt {

struct rt : public motis::module::module {
  std::string name() const override { return "rt"; }

  boost::program_options::options_description desc() override;
  void print(std::ostream& out) const override {}
  bool empty_config() const override { return true; }

  void init(motis::module::registry&) override;

private:
  motis::module::msg_ptr handle_messages(motis::module::msg_ptr const&);
};

}  // namespace rt
}  // namespace motis
