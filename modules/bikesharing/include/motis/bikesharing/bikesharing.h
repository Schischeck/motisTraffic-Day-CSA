#pragma once

#include <vector>

#include "motis/module/module.h"

namespace motis {
namespace bikesharing {

struct bikesharing : public motis::module::module {
  virtual ~bikesharing() {}

  virtual boost::program_options::options_description desc() override;
  virtual void print(std::ostream& out) const override;
  virtual bool empty_config() const override { return true; }

  virtual std::string name() const override { return "bikesharing"; }
  virtual void init() override;
  virtual std::vector<MsgContent> subscriptions() const override {
    return {};
  }
  virtual void on_msg(motis::module::msg_ptr, motis::module::sid,
                      motis::module::callback) override;

};

}  // namespace bikesharing
}  // namespace motis
