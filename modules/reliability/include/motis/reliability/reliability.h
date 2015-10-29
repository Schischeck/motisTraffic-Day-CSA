#pragma once

#include <vector>

#include "motis/module/module.h"

#include "motis/reliability/start_and_travel_distributions.h"
#include "motis/reliability/distributions_container.h"

namespace motis {
namespace reliability {
struct reliability : public motis::module::module {
  void init() override;

  virtual boost::program_options::options_description desc() override;
  virtual void print(std::ostream& out) const override;
  virtual bool empty_config() const override { return true; }

  virtual std::string name() const override { return "reliability"; }
  virtual std::vector<MsgContent> subscriptions() const override {
    return {MsgContent_ReliableRoutingRequest};
  }
  virtual void on_msg(motis::module::msg_ptr, motis::module::sid,
                      motis::module::callback) override;

  distributions_container::precomputed_distributions_container const&
  precomputed_distributions() const {
    return *precomputed_distributions_;
  }

  void send_message(motis::module::msg_ptr msg, motis::module::sid session,
                    motis::module::callback cb);

private:
  std::unique_ptr<distributions_container::precomputed_distributions_container>
      precomputed_distributions_;
  std::unique_ptr<start_and_travel_distributions> s_t_distributions_;

  void handle_routing_response(motis::module::msg_ptr,
                               boost::system::error_code,
                               motis::module::callback);
};
}  // namespace reliability
}  // namespace motis
