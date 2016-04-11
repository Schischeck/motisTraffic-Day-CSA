#pragma once

#include <memory>
#include <string>
#include <vector>

#include "boost/asio/io_service.hpp"

#include "motis/core/schedule/schedule.h"
#include "motis/module/controller.h"
#include "motis/module/module.h"
#include "motis/loader/loader_options.h"
#include "modules.h"

namespace motis {
namespace bootstrap {

struct motis_instance : public motis::module::controller {
  motis_instance();

  motis_instance(motis_instance const&) = delete;
  motis_instance& operator=(motis_instance const&) = delete;

  std::vector<motis::module::module*> modules() const;
  void init_schedule(motis::loader::loader_options const& dataset_opt);
  void init_modules(std::vector<std::string> const& modules);

  schedule_ptr schedule_;
  std::vector<std::unique_ptr<motis::module::module>> modules_;
};

using motis_instance_ptr = std::unique_ptr<motis_instance>;

}  // namespace bootstrap
}  // namespace motis
