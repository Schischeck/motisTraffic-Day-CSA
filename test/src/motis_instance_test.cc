#include "motis/test/motis_instance_test.h"

#include <system_error>

#include "conf/options_parser.h"

#include "motis/core/common/logging.h"

#include "motis/loader/parser_error.h"

#include "motis/module/context/motis_call.h"
#include "motis/module/context/motis_publish.h"
#include "motis/module/message.h"

using namespace motis::module;
using namespace motis::bootstrap;

namespace motis {
namespace test {

motis_instance_test::motis_instance_test(
    loader::loader_options const& dataset_opt,
    std::vector<std::string> const& modules,
    std::vector<std::string> const& modules_cmdline_opt)
    : instance_(std::make_unique<motis_instance>()) {
  auto modules_cmdline_opt_patched = modules_cmdline_opt;
  modules_cmdline_opt_patched.emplace_back("--ris.db_max_size=1048576");
  modules_cmdline_opt_patched.emplace_back("--ris.clear_db=true");

  std::vector<conf::configuration*> confs;
  for (auto const& module : instance_->modules()) {
    confs.push_back(module);
  }

  conf::options_parser parser(confs);
  parser.read_command_line_args(modules_cmdline_opt_patched);

  try {
    instance_->init_schedule(dataset_opt);
  } catch (loader::parser_error const& e) {
    LOG(logging::error) << "unable to parse schedule, problem at "
                        << e.filename_copy_ << ":" << e.line_number_;
    throw;
  }
  instance_->init_modules(modules);
}

msg_ptr motis_instance_test::call(msg_ptr const& msg) {
  return instance_->call(msg);
}

void motis_instance_test::publish(msg_ptr const& msg) {
  instance_->publish(msg);
}

msg_ptr motis_instance_test::call(std::string const& target) {
  return call(make_no_msg(target));
}

void motis_instance_test::publish(std::string const& target) {
  publish(make_no_msg(target));
}

std::function<module::msg_ptr(module::msg_ptr const&)>
motis_instance_test::msg_sink(std::vector<module::msg_ptr>* vec) {
  return [vec](module::msg_ptr const& m) -> module::msg_ptr {
    vec->push_back(m);
    return nullptr;
  };
}

}  // namespace test
}  // namespace motis
