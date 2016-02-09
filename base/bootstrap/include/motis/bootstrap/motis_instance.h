#pragma once

#include <memory>
#include <vector>
#include <string>

#include "boost/asio/io_service.hpp"

#include "motis/core/schedule/schedule.h"
#include "motis/module/module.h"
#include "motis/module/dispatcher.h"
#include "motis/module/callbacks.h"
#include "motis/loader/loader_options.h"
#include "modules.h"

namespace motis {
namespace bootstrap {

struct motis_instance : public motis::module::dispatcher {
  motis_instance();

  motis_instance(motis_instance const&) = delete;
  motis_instance(motis_instance&&) = delete;

  motis_instance& operator=(motis_instance const&) = delete;
  motis_instance& operator=(motis_instance&&) = delete;

  std::vector<motis::module::module*> modules() const;
  void init_schedule(motis::loader::loader_options const& dataset_opt);
  void init_modules(std::vector<std::string> const& modules);

  void run();

  boost::asio::io_service thread_pool_;
  schedule_ptr schedule_;
  motis::module::context module_context_;
  motis::module::msg_handler dispatch_fun_;
  std::vector<std::unique_ptr<motis::module::module>> modules_;
};

}  // namespace bootstrap
}  // namespace motis
