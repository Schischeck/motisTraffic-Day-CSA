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
  virtual std::vector<MsgContent> subscriptions() const override {
    return {MsgContent_ReliableRoutingRequest};
  }
  virtual void on_msg(motis::module::msg_ptr, motis::module::sid,
                      motis::module::callback) override;

private:
  typedef std::function<void(routing::RoutingResponse const*,
                             std::vector<float>, boost::system::error_code)>
      rating_response_cb;

  void find_connections(routing::RoutingRequest const*,
                        motis::module::sid session_id, rating_response_cb);
  void handle_routing_response(motis::module::msg_ptr,
                               boost::system::error_code,
                               rating_response_cb cb);
};

}  // namespace reliability
}  // namespace motis
