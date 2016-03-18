#pragma once

#include <ctime>
#include <string>

#include "boost/program_options.hpp"

#include "motis/module/module.h"
#include "motis/ris/mode/base_mode.h"

namespace po = boost::program_options;

namespace motis {
namespace ris {
enum class mode_t { LIVE, SIMULATION, TEST };

struct ris : public motis::module::module {
  ris();
  virtual ~ris() = default;

  boost::program_options::options_description desc() override;
  void print(std::ostream& out) const override;

  virtual void init() override;
  virtual void init_async() override;
  std::string name() const override { return "ris"; }
  std::vector<MsgContent> subscriptions() const override {
    return {MsgContent_RISForwardTimeRequest, MsgContent_HTTPRequest};
  }

  void on_msg(motis::module::msg_ptr, motis::module::sid,
              motis::module::callback) override;

  // TODO remove these when operations are available
  template <typename... T>
  void dispatch2(T&&... args) {
    dispatch(std::forward<T>(args)...);
  }
  auto get_thread_pool2() -> decltype(get_thread_pool()) {
    return get_thread_pool();
  }
  auto synced_sched2() -> decltype(synced_sched<schedule_access::RO>()) {
    return synced_sched<schedule_access::RO>();
  }

  // config
  mode_t mode_;
  int update_interval_;
  std::string input_folder_;
  int max_days_;

  std::time_t sim_init_start_;
  std::time_t sim_init_end_;

private:
  std::unique_ptr<mode::base_mode> active_mode_;
};

}  // namespace ris
}  // namespace motis
