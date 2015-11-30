#pragma once

#include <memory>
#include <string>
#include <vector>

#include "boost/system/system_error.hpp"

#include "motis/bootstrap/motis_instance.h"
#include "motis/core/common/util.h"
#include "motis/module/message.h"

namespace motis {
namespace bootstrap {

std::unique_ptr<motis_instance> launch_motis(
    std::string const& dataset, std::string const& schedule_begin,
    std::vector<std::string> const& modules) {
  auto instance = make_unique<motis_instance>();
  instance->init_schedule({dataset, false, schedule_begin, 2});
  instance->init_modules(modules);
  return instance;
}

module::msg_ptr send(std::unique_ptr<motis_instance> const& instance,
                     module::msg_ptr request) {
  module::msg_ptr response;
  boost::system::error_code ec;
  instance->on_msg(request, 1,
                   [&](module::msg_ptr r, boost::system::error_code e) {
                     ec = e;
                     response = r;
                   });
  instance->thread_pool_.reset();
  instance->thread_pool_.run();

  if (ec) {
    throw std::runtime_error(std::string(ec.category().name()) + " - " +
                             ec.category().message(ec.value()));
  }

  return response;
}

}  // namespace bootstrap
}  // namespace motis
