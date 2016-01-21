#pragma once

#include <memory>
#include <vector>

#include "motis/module/module.h"

#include "motis/core/schedule/schedule.h"

namespace motis {
namespace connectionchecker {

struct connectionchecker : public motis::module::module {
  void init() override;

  virtual boost::program_options::options_description desc() override;
  virtual void print(std::ostream& out) const override;
  virtual bool empty_config() const override { return true; }

  virtual std::string name() const override { return "connectionchecker"; }
  virtual std::vector<MsgContent> subscriptions() const override {
    return {MsgContent_RoutingResponse};
  }
  virtual void on_msg(motis::module::msg_ptr, motis::module::sid,
                      motis::module::callback) override;
};

}  // namespace connectionchecker
}  // namespace motis
