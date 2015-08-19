#pragma once

#include <vector>

#include "motis/module/module.h"

namespace motis {
namespace reliability {

struct reliability : public motis::module::module {
  reliability();
  virtual ~reliability() {}

  bool initialize();

  virtual boost::program_options::options_description desc() override;
  virtual void print(std::ostream& out) const override;

  virtual std::string name() const override { return "reliability"; }
  virtual std::vector<MsgContent> subscriptions() const override { return {}; }
  virtual void on_msg(motis::module::msg_ptr, motis::module::sid,
                      motis::module::callback) override;
};

}  // namespace reliability
}  // namespace motis
