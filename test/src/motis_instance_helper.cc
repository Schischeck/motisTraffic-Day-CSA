#include "motis/test/motis_instance_helper.h"

#include "boost/system/system_error.hpp"

#include "conf/options_parser.h"

#include "motis/bootstrap/motis_instance.h"
#include "motis/module/message.h"

using namespace motis::module;

using motis::bootstrap::motis_instance;
using boost::system::error_code;

namespace motis {
namespace test {

msg_ptr test_instance::call(msg_ptr const& msg) {
  msg_ptr response;
  error_code ec;
  motis->on_msg(msg, [&](msg_ptr r, error_code e) {
    response = r;
    ec = e;
  });
  ios.run();
  ios.reset();

  if (ec) {
    throw boost::system::system_error(ec);
  }
  return response;
}

msg_ptr test_instance::call(std::string const& t) {
  MessageCreator fbb;
  fbb.CreateAndFinish(MsgContent_MotisNoMessage,
                      CreateMotisNoMessage(fbb).Union(), t);
  return call(make_msg(fbb));
}

test_instance_ptr launch_motis(
    std::string const& dataset, std::string const& schedule_begin,
    std::vector<std::string> const& modules,
    std::vector<std::string> const& modules_cmdline_opt) {
  auto instance = std::make_unique<test_instance>();
  instance->motis = std::make_unique<motis_instance>(instance->ios);

  std::vector<conf::configuration*> confs;
  for (auto const& module : instance->motis->modules()) {
    confs.push_back(module);
  }

  conf::options_parser parser(confs);
  std::vector<std::string> opt(begin(modules_cmdline_opt),
                               end(modules_cmdline_opt));
  opt.push_back("--routing.label_store_size=32000");
  parser.read_command_line_args(opt);

  instance->motis->init_schedule(
      {dataset, false, true, false, true, schedule_begin, 2});
  instance->motis->init_modules(modules);

  return instance;
}

}  // namespace test
}  // namespace motis
