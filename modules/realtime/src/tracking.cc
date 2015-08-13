#include "motis/realtime/tracking.h"
#include "motis/realtime/realtime_schedule.h"

namespace motis {
namespace realtime {

tracking::tracking(realtime_schedule& rts)
    : _rts(rts), _in_msg_file("tracking-in-messages.txt") {}

void tracking::in_message(const message_class& msg) {
  msg.write_to_stream(_in_msg_file);
  _in_msg_file.flush();
}

}  // namespace realtime
}  // namespace motis