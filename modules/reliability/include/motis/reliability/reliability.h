#pragma once

#include "motis/module/module.h"

namespace motis {
namespace reliability {

struct reliability : public motis::module::module {
  reliability();
  virtual ~reliability() {}

  virtual boost::program_options::options_description desc() override;
  virtual void print(std::ostream& out) const override;

  virtual std::string name() const override { return "reliability"; }
  virtual std::vector<MsgContent> subscriptions() const { return {}; }
  virtual motis::module::msg_ptr on_msg(motis::module::msg_ptr const&,
                                        motis::module::sid) override;
};

}  // namespace reliability
}  // namespace motis
