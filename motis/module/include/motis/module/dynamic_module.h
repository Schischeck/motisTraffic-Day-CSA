#pragma once

#include <string>
#include <memory>
#include <vector>

#include "motis/core/schedule/Schedule.h"

#include "motis/module/module.h"

namespace motis {
namespace module {

struct dynamic_module {
  dynamic_module(std::string const& path, td::Schedule* schedule);
  ~dynamic_module();

  std::shared_ptr<motis::module::module> module_;
  void* lib_;
};

std::vector<dynamic_module> modules_from_folder(std::string const& path,
                                                td::Schedule* schedule);

}  // module
}  // motis