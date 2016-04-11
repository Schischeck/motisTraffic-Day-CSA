#pragma once

#include "motis/module/message.h"
#include "motis/module/registry.h"
#include "motis/ris/mode/base_mode.h"
#include "motis/ris/ris.h"

namespace motis {
namespace ris {
namespace mode {

struct simulation_mode final : public base_mode {
  simulation_mode(config* conf) : base_mode(conf) {}

  void init(motis::module::registry& r) override {
    base_mode::init(r);
    r.register_op("/ris/forward", [this](motis::module::msg_ptr const& msg) {
      handle_forward_request(msg);
    });
  }

  void init_async() override {
    base_mode::init_async();
    forward(conf_->sim_init_time_);
  }

  void handle_forward_request(motis::module::msg_ptr const& msg) {
    auto req = motis_content(RISForwardTimeRequest, msg);
    forward(req->new_time());
  }
};

}  // namespace mode
}  // namespace ris
}  // namespace motis
