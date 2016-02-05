#include "motis/bootstrap/motis_instance.h"

#include <algorithm>
#include <exception>
#include <future>

#include "motis/core/common/logging.h"
#include "motis/loader/loader.h"
#include "motis/module/callbacks.h"

#include "modules.h"

using namespace motis::module;
using namespace motis::logging;
namespace p = std::placeholders;

namespace motis {
namespace bootstrap {

motis_instance::motis_instance(boost::asio::io_service* ios)
    : dispatcher(ios ? *ios : thread_pool_),
      dispatch_fun_(
          std::bind(&dispatcher::on_msg, this, p::_1, p::_2, p::_3, p::_4)),
      modules_(build_modules()) {}

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
}

void motis_instance::init_modules(std::vector<std::string> const& modules) {
  module_context_ = {schedule_.get(), &ios_, &thread_pool_, &send_fun_,
                     &dispatch_fun_};

  for (auto const& module : modules_) {
    if (std::find(begin(modules), end(modules), module->name()) ==
        end(modules)) {
      continue;
    }

    dispatcher::modules_.push_back(module.get());
    add_module(module.get());

    try {
      module->init_(&module_context_);
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

  for (auto const& module : dispatcher::modules_) {
    std::promise<void> promise;
    callback cb = [&promise](msg_ptr, boost::system::error_code ec) {
      if (ec) {
        promise.set_exception(
            std::make_exception_ptr(boost::system::system_error(ec)));
      } else {
        promise.set_value();
      }
    };
    module->init_async(cb);
    thread_pool_.run();
    thread_pool_.reset();

    auto future = promise.get_future();
    future.wait();
    try {
      future.get();
    } catch (std::exception const& e) {
      LOG(emrg) << "module " << module->name()
                << ": unhandled init_async error: " << e.what();
      throw;
    } catch (...) {
      LOG(emrg) << "module " << module->name()
                << "unhandled unknown init_async error";
      throw;
    }
  }
}

void motis_instance::run() { thread_pool_.run(); }

}  // namespace bootstrap
}  // namespace motis
