#pragma once

#include <memory>
#include <vector>

#include "motis/module/module.h"

#include "motis/core/schedule/schedule.h"

#include "motis/reliability/start_and_travel_distributions.h"
#include "motis/reliability/distributions_container.h"
#include "motis/reliability/search/simple_connection_graph_optimizer.h"

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

  synced_schedule<RO> synced_sched() { return module::synced_sched<RO>(); }

  std::unique_ptr<search::connection_graph_search::simple_optimizer> optimizer_;

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
