#pragma once

#include "motis/module/module.h"

#include "motis/routing/memory_manager.h"
#include "motis/routing/label.h"

namespace motis {
namespace routing {

struct routing : public motis::module::module {
  routing();
  virtual ~routing() {}

  virtual boost::program_options::options_description desc() override;
  virtual void print(std::ostream& out) const override;

  virtual std::string name() const override { return "routing"; }
  virtual std::vector<MsgContent> subscriptions() const {
    return {MsgContent_RoutingRequest};
  }
  virtual motis::module::msg_ptr on_msg(motis::module::msg_ptr const&,
                                        motis::module::sid) override;

  memory_manager<label> label_store_;
};

}  // namespace routing
}  // namespace motis
