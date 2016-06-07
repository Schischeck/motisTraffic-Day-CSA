#pragma once

#include <memory>
#include <vector>

#include "motis/module/module.h"

namespace motis {
namespace rt {

struct delay_propagator;

struct rt : public motis::module::module {
  rt();
  ~rt() override;

  std::string name() const override { return "rt"; }

  boost::program_options::options_description desc() override;
  void print(std::ostream&) const override {}
  bool empty_config() const override { return true; }

  void init(motis::module::registry&) override;

private:
  motis::module::msg_ptr handle_messages(motis::module::msg_ptr const&);

  std::unique_ptr<delay_propagator> propagator_;
};

}  // namespace rt
}  // namespace motis
