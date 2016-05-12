#pragma once

#include <functional>
#include <string>
#include <vector>

#include "gtest/gtest.h"

#include "motis/bootstrap/motis_instance.h"
#include "motis/loader/loader_options.h"

namespace motis {
namespace test {

struct motis_instance_test : public ::testing::Test {
  motis_instance_test(loader::loader_options const&,
                      std::vector<std::string> const& modules = {},
                      std::vector<std::string> const& modules_cmdline_opt = {});

  template <typename F>
  void subscribe(std::string const& topic, F&& func) {
    instance_->subscribe(topic, std::forward<F>(func));
  }

  template <typename Fn>
  void run(Fn&& fn) {
    instance_->run(fn);
  }

  module::msg_ptr call(std::string const& target);
  module::msg_ptr call(module::msg_ptr const&);

  std::function<module::msg_ptr(module::msg_ptr const&)> msg_sink(
      std::vector<module::msg_ptr>*);

  schedule& sched() { return *instance_->sched_; }

private:
  bootstrap::motis_instance_ptr instance_;
};

}  // namespace test
}  // namespace motis
