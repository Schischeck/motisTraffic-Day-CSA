#include "motis/test/schedule/invalid_realtime.h"

#include "motis/core/schedule/schedule.h"
#include "motis/core/access/time_access.h"
#include "motis/ris/risml/risml_parser.h"

using namespace flatbuffers;
using namespace motis::module;
using namespace motis::ris;

namespace motis {
namespace test {
namespace schedule {
namespace invalid_realtime {

msg_ptr get_trip_conflict_ris_message() {
  return motis::ris::risml::file_to_msg(
      "test/schedule/invalid_realtime/risml/trip_conflict.xml");
}

msg_ptr get_ts_conflict_ris_message() {
  return motis::ris::risml::file_to_msg(
      "test/schedule/invalid_realtime/risml/ts_conflict.xml");
}

motis::module::msg_ptr get_additional_ris_message() {
  return motis::ris::risml::file_to_msg(
      "test/schedule/invalid_realtime/risml/additional.xml");
}

motis::module::msg_ptr get_cancel_ris_message() {
  return motis::ris::risml::file_to_msg(
      "test/schedule/invalid_realtime/risml/cancel.xml");
}

motis::module::msg_ptr get_reroute_ris_message() {
  return motis::ris::risml::file_to_msg(
      "test/schedule/invalid_realtime/risml/reroute.xml");
}

}  // namespace invalid_realtime
}  // namespace schedule
}  // namespace test
}  // namespace motis
