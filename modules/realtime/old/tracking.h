#pragma once

#include <iostream>
#include <fstream>

#include "motis/realtime/message_classes.h"

namespace motis {
namespace realtime {

class realtime_schedule;

class tracking {
public:
  tracking(realtime_schedule& rts);

  void in_message(const message_class& msg);

  realtime_schedule& _rts;

private:
  std::ofstream _in_msg_file;
};

}  // namespace realtime
}  // namespace motis
