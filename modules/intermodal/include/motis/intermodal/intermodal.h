#pragma once

#include <vector>

#include "motis/module/module.h"

namespace motis {
namespace intermodal {

struct intermodal : public motis::module::module {
  intermodal() = default;
  ~intermodal() = default;

  virtual boost::program_options::options_description desc() override;
  virtual void print(std::ostream&) const override {}
  virtual bool empty_config() const override { return true; }

  virtual std::string name() const override { return "intermodal"; }
  virtual void init() override {}
  virtual std::vector<MsgContent> subscriptions() const override { return {}; }
  virtual void on_msg(motis::module::msg_ptr, motis::module::sid,
                      motis::module::callback) override;
};

}  // namespace intermodal
}  // namespace motis
