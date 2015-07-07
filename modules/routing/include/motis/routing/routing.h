#pragma once

#include "motis/module/module.h"

namespace motis {
namespace routing {

struct routing : public motis::module::module {
  virtual boost::program_options::options_description desc() override;
  virtual void print(std::ostream& out) const override;

  virtual std::string name() const override { return "routing"; }
  virtual std::vector<MsgContent> subscriptions() const { return {}; }
  virtual motis::module::msg_ptr on_msg(motis::module::msg_ptr const&,
                                        motis::module::sid) override;
};

}  // namespace routing
}  // namespace motis
