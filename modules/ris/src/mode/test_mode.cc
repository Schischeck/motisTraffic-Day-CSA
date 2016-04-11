#include "motis/ris/mode/test_mode.h"

#include <algorithm>

#include "ctx/future.h"
#include "parser/file.h"

#include "motis/core/common/logging.h"
#include "motis/module/motis_publish.h"
#include "motis/ris/detail/find_new_files.h"
#include "motis/ris/detail/pack_msgs.h"
#include "motis/ris/ris.h"
#include "motis/ris/ris_message.h"
#include "motis/ris/risml/risml_parser.h"

using namespace motis::logging;
using namespace motis::module;
using namespace motis::ris::detail;
using namespace motis::ris::risml;

namespace motis {
namespace ris {
namespace mode {

void test_mode::init_async() {
  auto files = find_new_files(conf_->input_folder_, ".xml", {});
  std::sort(begin(files), end(files));

  std::vector<ris_message> parsed_messages;
  manual_timer timer("RISML parse " + std::to_string(files.size()) + " xmls");
  for (auto const& xml_file : files) {
    std::vector<parser::buffer> bufs;
    bufs.emplace_back(parser::file(xml_file.c_str(), "r").content());
    for (auto&& msg : parse_xmls(std::move(bufs))) {
      parsed_messages.emplace_back(std::move(msg));
    }
  }
  timer.stop_and_print();

  ctx::await_all(motis_publish(pack_msgs(parsed_messages)));

  auto new_time = std::max(begin(parsed_messages), end(parsed_messages),
                           [](ris_message const& lhs, ris_message const& rhs) {
                             return lhs.timestamp < rhs.timestamp;
                           });
  system_time_changed(new_time);
}

}  // namespace mode
}  // namespace ris
}  // namespace motis
