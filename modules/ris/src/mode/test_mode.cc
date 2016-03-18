#include "motis/ris/mode/test_mode.h"

#include "parser/file.h"

#include "motis/core/common/logging.h"
#include "motis/ris/detail/find_new_files.h"
#include "motis/ris/detail/pack_msgs.h"
#include "motis/ris/error.h"
#include "motis/ris/ris.h"
#include "motis/ris/ris_message.h"
#include "motis/ris/risml_parser.h"

using namespace motis::logging;
using namespace motis::module;
using namespace motis::ris::detail;

namespace motis {
namespace ris {
namespace mode {

void test_mode::init_async() {
  auto files = find_new_files(module_->input_folder_, ".xml", {});
  std::sort(begin(files), end(files));

  std::vector<ris_message> parsed_messages;
  auto str = "RISML parse " + std::to_string(files.size()) + " xmls";
  manual_timer timer(str.c_str());
  for (auto const& xml_file : files) {
    std::vector<parser::buffer> bufs;
    bufs.emplace_back(parser::file(xml_file.c_str(), "r").content());
    for (auto&& msg : parse_xmls(std::move(bufs))) {
      parsed_messages.emplace_back(std::move(msg));
    }
  }
  timer.stop_and_print();

  module_->dispatch2(pack_msgs(parsed_messages));
}

void test_mode::on_msg(msg_ptr, sid, callback cb) {
  return cb({}, error::unexpected_message);
}

}  // namespace mode
}  // namespace ris
}  // namespace motis
