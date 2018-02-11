#include "motis/test/schedule/simple_realtime.h"

#include "motis/ris/risml/risml_parser.h"

namespace motis {
namespace test {
namespace schedule {
namespace simple_realtime {

motis::module::msg_ptr get_ris_message() {
  return motis::ris::risml::file_to_msg(
      "test/schedule/simple_realtime/risml/delays.xml");
}

}  // namespace simple_realtime
}  // namespace schedule
}  // namespace test
}  // namespace motis
