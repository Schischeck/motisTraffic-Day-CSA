#include "motis/bootstrap/motis_instance.h"

#include <chrono>
#include <algorithm>
#include <atomic>
#include <exception>
#include <future>
#include <thread>

#include "ctx/future.h"

#include "motis/core/common/logging.h"
#include "motis/module/context/motis_call.h"
#include "motis/module/context/motis_publish.h"
#include "motis/loader/loader.h"

#include "modules.h"

using namespace motis::module;
using namespace motis::logging;

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

std::vector<std::string> motis_instance::module_names() const {
  std::vector<std::string> s;
  for (auto const& module : modules_) {
    s.push_back(module->name());
  }
  return s;
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
  publish("/init");
}

msg_ptr motis_instance::call(std::string const& target) {
  return call(make_no_msg(target));
}

msg_ptr motis_instance::call(msg_ptr const& msg) {
  std::exception_ptr e;
  msg_ptr response;

  run([&]() {
    try {
      response = motis_call(msg)->val();
    } catch (...) {
      e = std::current_exception();
    }
  });
  ios_.run();
  ios_.reset();

  if (e) {
    std::rethrow_exception(e);
  }

  return response;
}

void motis_instance::publish(std::string const& target) {
  publish(make_no_msg(target));
}

void motis_instance::publish(msg_ptr const& msg) {
  std::exception_ptr e;

  run([&]() {
    try {
      ctx::await_all(motis_publish(msg));
    } catch (...) {
      e = std::current_exception();
    }
  });
  ios_.run();
  ios_.reset();

  if (e) {
    std::rethrow_exception(e);
  }
}

}  // namespace bootstrap
}  // namespace motis
