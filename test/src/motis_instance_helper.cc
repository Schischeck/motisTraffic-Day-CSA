#include "motis/test/motis_instance_helper.h"

#include "boost/system/system_error.hpp"

#include "conf/options_parser.h"

#include "motis/module/message.h"

using namespace motis::module;
using namespace motis::bootstrap;
using boost::system::error_code;

namespace motis {
namespace test {

msg_ptr call(motis_instance_ptr const& instance, msg_ptr const& msg) {
  msg_ptr response;
  error_code ec;
  instance->on_msg(msg, [&](msg_ptr r, error_code e) {
    response = r;
    ec = e;
  });
  instance->ios_.run();
  instance->ios_.reset();

  if (ec) {
    throw boost::system::system_error(ec);
  }
  return response;
}

msg_ptr call(motis_instance_ptr const& instance, std::string const& t) {
  MessageCreator fbb;
  fbb.CreateAndFinish(MsgContent_MotisNoMessage,
                      CreateMotisNoMessage(fbb).Union(), t);
  return call(instance, make_msg(fbb));
}

motis_instance_ptr launch_motis(
    std::string const& dataset, std::string const& schedule_begin,
    std::vector<std::string> const& modules,
    std::vector<std::string> const& modules_cmdline_opt) {
  auto instance = std::make_unique<motis_instance>();

  std::vector<conf::configuration*> confs;
  for (auto const& module : instance->modules()) {
    confs.push_back(module);
  }

  conf::options_parser parser(confs);
  std::vector<std::string> opt(begin(modules_cmdline_opt),
                               end(modules_cmdline_opt));
  opt.push_back("--routing.label_store_size=32000");
  parser.read_command_line_args(opt);

  instance->init_schedule(
      {dataset, false, true, false, true, schedule_begin, 2});
  instance->init_modules(modules);

  return instance;
}

}  // namespace test
}  // namespace motis
