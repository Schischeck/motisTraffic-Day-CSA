#pragma once

#include "motis/module/module.h"

#include "motis/routing/memory_manager.h"

namespace motis {
namespace routing {

struct routing : public motis::module::module {
  routing();
  std::string name() const override { return "routing"; }

  boost::program_options::options_description desc() override;
  void print(std::ostream& out) const override;
  bool empty_config() const override { return false; }

  void init(motis::module::registry&) override;

private:
  motis::module::msg_ptr route(motis::module::msg_ptr const&);

  std::size_t label_bytes_;
  std::unique_ptr<memory_manager> label_store_;
};

}  // namespace routing
}  // namespace motis
