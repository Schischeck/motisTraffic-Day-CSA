#pragma once

#include <vector>

#include "motis/module/module.h"

namespace motis {
namespace intermodal {

struct intermodal : public motis::module::module {
  intermodal();
  ~intermodal();

  virtual boost::program_options::options_description desc() override;
  virtual void print(std::ostream& out) const override;
  virtual bool empty_config() const override { return true; }

  virtual std::string name() const override { return "intermodal"; }
  virtual void init() override;
  virtual std::vector<MsgContent> subscriptions() const override {
    return {MsgContent_IntermodalRequest};
  }
  virtual void on_msg(motis::module::msg_ptr, motis::module::sid,
                      motis::module::callback) override;

  struct impl;
  std::unique_ptr<impl> impl_;
};

}  // namespace intermodal
}  // namespace motis
