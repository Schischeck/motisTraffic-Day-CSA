#include "motis/test/motis_instance_helper.h"

#include "boost/system/system_error.hpp"

#include "conf/options_parser.h"

#include "motis/bootstrap/motis_instance.h"
#include "motis/core/common/util.h"
#include "motis/module/message.h"

using motis::bootstrap::motis_instance;

namespace motis {
namespace test {

std::unique_ptr<motis_instance> launch_motis(
    std::string const& dataset, std::string const& schedule_begin,
    std::vector<std::string> const& modules,
    std::vector<std::string> const& modules_cmdline_opt) {
  auto instance = make_unique<motis_instance>();

  std::vector<conf::configuration*> confs;
  for (auto const& module : instance->modules()) {
    confs.push_back(module);
  }

  conf::options_parser parser(confs);
  std::vector<std::string> opt(begin(modules_cmdline_opt),
                               end(modules_cmdline_opt));
  opt.push_back("--routing.max_label_count=1000");
  parser.read_command_line_args(opt);

  instance->init_schedule({dataset, false, true, true, schedule_begin, 2});
  instance->init_modules(modules);

  return instance;
}

module::msg_ptr send(std::unique_ptr<motis_instance> const& instance,
                     module::msg_ptr request) {
  module::msg_ptr response;
  boost::system::error_code ec;
  instance->on_msg(request,
                   0, [&](module::msg_ptr r, boost::system::error_code e) {
                     ec = e;
                     response = r;
                   }, false);
  instance->thread_pool_.reset();
  instance->thread_pool_.run();

  if (ec) {
    throw boost::system::system_error(ec);
  }

  return response;
}

}  // namespace test
}  // namespace motis
