#pragma once

#include <memory>
#include <vector>

#include "motis/module/module.h"
#include "motis/rt/statistics.h"

namespace motis {

struct trip;

namespace rt {

struct delay_propagator;

struct rt : public motis::module::module {
  rt();
  ~rt() override;

  std::string name() const override { return "rt"; }

  boost::program_options::options_description desc() override;
  void print(std::ostream&) const override {}
  bool empty_config() const override { return true; }

  void init(motis::module::registry&) override;

private:
  trip const* get_trip_fuzzy(schedule const& sched,
                             ris::DelayMessage const* msg);

  motis::module::msg_ptr on_message(motis::module::msg_ptr const& msg);
  motis::module::msg_ptr on_system_time_change(
      motis::module::msg_ptr const& msg);

  motis::module::msg_ptr handle_messages(motis::module::msg_ptr const&);
  void add_to_propagator(schedule const& sched, ris::DelayMessage const* msg);

  std::unique_ptr<delay_propagator> propagator_;
  statistics stats_;
};

}  // namespace rt
}  // namespace motis
