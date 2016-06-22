#pragma once

#include <memory>
#include <vector>

#include "motis/protocol/Message_generated.h"

#include "motis/module/module.h"

#include "motis/core/schedule/schedule.h"
#include "motis/core/schedule/synced_schedule.h"

#include "motis/reliability/distributions/distributions_container.h"
#include "motis/reliability/distributions/start_and_travel_distributions.h"
#include "motis/reliability/search/cg_optimizer.h"

namespace motis {
namespace reliability {

struct reliability : public motis::module::module {
  reliability();

  std::string name() const override { return "reliability"; }
  boost::program_options::options_description desc() override;
  void print(std::ostream& out) const override;
  bool empty_config() const override { return false; }

  void init(motis::module::registry&) override;

  distributions_container::container const& precomputed_distributions() const {
    return *precomputed_distributions_;
  }

  start_and_travel_distributions const& s_t_distributions() const {
    return *s_t_distributions_;
  }

  synced_schedule<RO> synced_sched() { return module::synced_sched<RO>(); }

  std::unique_ptr<distributions_container::container>
      precomputed_distributions_;
  std::unique_ptr<start_and_travel_distributions> s_t_distributions_;

private:
  motis::module::msg_ptr routing_request(motis::module::msg_ptr const&);

  motis::module::msg_ptr realtime_update(motis::module::msg_ptr const&);

  bool read_distributions_;
  std::vector<std::string> distributions_folders_;
  std::string hotels_file_;
  unsigned max_bikesharing_duration_;
  bool pareto_filtering_for_bikesharing_;
};  // struct reliability
}  // namespace reliability
}  // namespace motis
