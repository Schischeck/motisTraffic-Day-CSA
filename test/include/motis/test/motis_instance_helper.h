#pragma once

#include <memory>
#include <string>
#include <vector>

#include "boost/asio/io_service.hpp"

#include "motis/bootstrap/motis_instance.h"
#include "motis/module/message.h"

namespace motis {
namespace test {

struct test_instance {
  template <typename F>
  void subscribe(std::string const& topic, F&& fn) {
    motis->subscribe(topic, std::forward<F>(fn));
  }

  motis::module::msg_ptr call(std::string const& target);
  motis::module::msg_ptr call(motis::module::msg_ptr const& msg);

  boost::asio::io_service ios;
  std::unique_ptr<motis::bootstrap::motis_instance> motis;
};

using test_instance_ptr = std::unique_ptr<test_instance>;

test_instance_ptr launch_motis(
    std::string const& dataset, std::string const& schedule_begin,
    std::vector<std::string> const& modules,
    std::vector<std::string> const& modules_cmdline_opt = {});

inline std::function<void(motis::module::msg_ptr const&)> msg_sink(
    std::vector<motis::module::msg_ptr>* vec) {
  return [vec](motis::module::msg_ptr const& m) { vec->push_back(m); };
}

}  // namespace test
}  // namespace motis
