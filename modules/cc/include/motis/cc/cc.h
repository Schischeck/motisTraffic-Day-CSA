#pragma once

#include "motis/module/module.h"

namespace motis {
namespace cc {

struct cc : public motis::module::module {
  std::string name() const override { return "cc"; }

  boost::program_options::options_description desc() override;
  void print(std::ostream&) const override {}
  bool empty_config() const override { return true; }

  void init(motis::module::registry&) override;

private:
  motis::module::msg_ptr check_journey(motis::module::msg_ptr const&) const;
};

}  // namespace cc
}  // namespace motis
