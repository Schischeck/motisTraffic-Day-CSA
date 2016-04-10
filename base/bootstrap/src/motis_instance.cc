#include "motis/bootstrap/motis_instance.h"

#include <chrono>
#include <algorithm>
#include <atomic>
#include <exception>
#include <future>
#include <thread>

#include "motis/core/common/logging.h"
#include "motis/loader/loader.h"

#include "modules.h"

using namespace motis::module;
using namespace motis::logging;
namespace p = std::placeholders;

namespace motis {
namespace bootstrap {

motis_instance::motis_instance() : modules_(build_modules()) {}

std::vector<motis::module::module*> motis_instance::modules() const {
  std::vector<motis::module::module*> m;
  for (auto& module : modules_) {
    m.push_back(module.get());
  }
  return m;
}

void motis_instance::init_schedule(
    motis::loader::loader_options const& dataset_opt) {
  schedule_ = loader::load_schedule(dataset_opt);
  sched_ = schedule_.get();
}

void motis_instance::init_modules(std::vector<std::string> const& modules) {
  for (auto const& module : modules_) {
    if (std::find(begin(modules), end(modules), module->name()) ==
        end(modules)) {
      continue;
    }

    try {
      module->set_context(*schedule_, ios_);
      module->init(registry_);
    } catch (std::exception const& e) {
      LOG(emrg) << "module " << module->name()
                << ": unhandled init error: " << e.what();
      throw;
    } catch (...) {
      LOG(emrg) << "module " << module->name()
                << "unhandled unknown init error";
      throw;
    }
  }
}

}  // namespace bootstrap
}  // namespace motis
