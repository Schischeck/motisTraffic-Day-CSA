#pragma once

#include <string>
#include <memory>
#include <vector>

#include "motis/core/schedule/schedule.h"

#include "motis/module/module.h"

namespace motis {
namespace module {

struct dynamic_module {
  dynamic_module(dynamic_module const&) = delete;
  dynamic_module& operator=(dynamic_module const&) = delete;

  dynamic_module(dynamic_module&& module);
  dynamic_module& operator=(dynamic_module&&);

  dynamic_module(std::string const& path, motis::schedule* schedule);
  ~dynamic_module();

  std::shared_ptr<motis::module::module> module_;
  void* lib_;
};

std::vector<dynamic_module> modules_from_folder(std::string const& path,
                                                motis::schedule* schedule);

}  // module
}  // motis
