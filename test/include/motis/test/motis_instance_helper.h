#pragma once

#include <memory>
#include <string>
#include <vector>

#include "motis/module/message.h"
#include "motis/bootstrap/motis_instance.h"

namespace motis {
namespace test {

bootstrap::motis_instance_ptr launch_motis(
    std::string const& dataset, std::string const& schedule_begin,
    std::vector<std::string> const& modules,
    std::vector<std::string> const& modules_cmdline_opt = {});

template <typename F>
void subscribe(bootstrap::motis_instance_ptr const& instance,
               std::string const& topic, F&& func) {
  instance->subscribe(topic, std::forward<F>(func));
}

module::msg_ptr call(bootstrap::motis_instance_ptr const&,
                     std::string const& target);
module::msg_ptr call(bootstrap::motis_instance_ptr const&,
                     module::msg_ptr const&);

std::function<module::msg_ptr(module::msg_ptr const&)> msg_sink(
    std::vector<module::msg_ptr>*);

}  // namespace test
}  // namespace motis
