#pragma once

#include <fstream>
#include <iostream>

#include "motis/realtime/messages.h"

namespace motis {
namespace realtime {

class realtime_schedule;

class tracking {
public:
  tracking(realtime_schedule& rts);

  void in_message(const message& msg);

  realtime_schedule& _rts;

private:
  std::ofstream _in_msg_file;
};

}  // namespace realtime
}  // namespace motis
