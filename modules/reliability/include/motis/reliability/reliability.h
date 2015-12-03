#pragma once

#include <memory>
#include <vector>

#include "motis/protocol/Message_generated.h"

#include "motis/module/module.h"

#include "motis/core/schedule/schedule.h"

#include "motis/reliability/start_and_travel_distributions.h"
#include "motis/reliability/distributions_container.h"
#include "motis/reliability/search/cg_optimizer.h"

namespace motis {
namespace reliability {
namespace search {
struct connection_graph;
}

struct reliability : public motis::module::module {
  void init() override;

  virtual boost::program_options::options_description desc() override;
  virtual void print(std::ostream& out) const override;
  virtual bool empty_config() const override { return true; }

  virtual std::string name() const override { return "reliability"; }
  virtual std::vector<MsgContent> subscriptions() const override {
    return {MsgContent_ReliableRoutingRequest,
            MsgContent_RealtimeDelayInfoResponse};
  }
  virtual void on_msg(motis::module::msg_ptr, motis::module::sid,
                      motis::module::callback) override;

  void handle_routing_request(
      motis::reliability::ReliableRoutingRequest const* req,
      motis::module::sid session_id, motis::module::callback cb);
  void handle_realtime_update(
      motis::realtime::RealtimeDelayInfoResponse const* update,
      motis::module::callback cb);

  distributions_container::precomputed_distributions_container const&
  precomputed_distributions() const {
    return *precomputed_distributions_;
  }

  start_and_travel_distributions const& s_t_distributions() const {
    return *s_t_distributions_;
  }

  void send_message(motis::module::msg_ptr msg, motis::module::sid session,
                    motis::module::callback cb);

  synced_schedule<RO> synced_sched() { return module::synced_sched<RO>(); }

private:
  std::unique_ptr<distributions_container::precomputed_distributions_container>
      precomputed_distributions_;
  std::unique_ptr<start_and_travel_distributions> s_t_distributions_;

  void handle_routing_response(motis::module::msg_ptr,
                               boost::system::error_code,
                               motis::module::callback);

  void handle_connection_graph_result(
      std::vector<std::shared_ptr<search::connection_graph> > const,
      motis::module::callback cb);
};
}  // namespace reliability
}  // namespace motis
