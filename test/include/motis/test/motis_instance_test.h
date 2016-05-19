#pragma once

#include <functional>
#include <string>
#include <vector>

#include "gtest/gtest.h"

#include "motis/core/access/time_access.h"
#include "motis/bootstrap/motis_instance.h"
#include "motis/loader/loader_options.h"

namespace motis {
namespace test {

struct motis_instance_test : public ::testing::Test {
  explicit motis_instance_test(
      loader::loader_options const&,
      std::vector<std::string> const& modules = {},
      std::vector<std::string> const& modules_cmdline_opt = {});

  template <typename F>
  void subscribe(std::string const& topic, F&& func) {
    instance_->subscribe(topic, std::forward<F>(func));
  }

  template <typename Fn>
  auto run(Fn&& fn) {
    return instance_->run(fn);
  }

  module::msg_ptr call(std::string const& target);
  module::msg_ptr call(module::msg_ptr const&);
  void publish(module::msg_ptr const&);

  std::function<module::msg_ptr(module::msg_ptr const&)> msg_sink(
      std::vector<module::msg_ptr>*);

  schedule const& sched() const { return *instance_->sched_; }
  schedule& sched() { return *instance_->sched_; }

  std::time_t unix_time(int hhmm, int day_idx = 0,
                        int timezone_offset = kDefaultTimezoneOffset) const {
    return motis::unix_time(sched(), hhmm, day_idx, timezone_offset);
  }

  template <typename Module>
  Module& get_module(std::string const module_name) {
    auto it = std::find_if(
        instance_->modules_.begin(), instance_->modules_.end(),
        [module_name](auto const& m) { return m->name() == module_name; });
    if (it == instance_->modules_.end()) {
      throw std::system_error();
    }
    return *reinterpret_cast<Module*>(it->get());
  }

private:
  bootstrap::motis_instance_ptr instance_;
};

}  // namespace test
}  // namespace motis
