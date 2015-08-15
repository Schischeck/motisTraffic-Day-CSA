#include "motis/realtime/message_output.h"

#include <fstream>
#include <string>

#include "motis/realtime/realtime_schedule.h"
#include "motis/core/common/logging.h"

namespace motis {
namespace realtime {

using namespace motis::logging;

message_output::message_output(realtime_schedule& rts) : _rts(rts) {
  std::time(&_base_time);
  std::tm t = *std::localtime(&_base_time);
  date_manager::date first_day = _rts._schedule.date_mgr.first_date();
  t.tm_mday = first_day.day;
  t.tm_mon = first_day.month - 1;
  t.tm_year = first_day.year - 1900;
  t.tm_hour = 0;
  t.tm_min = 0;
  t.tm_sec = 0;
  // t.tm_isdst = -1;
  _base_time = std::mktime(&t);
}

void message_output::add_delay(const delay_info* di) { _delays.push_back(di); }

void message_output::add_message(message_class const* msg) {
  _messages.push_back(msg);
}

void message_output::set_current_time(std::time_t t) { _current_time = t; }

void message_output::finish() {
  if (_rts.is_debug_mode()) {
    std::string name =
        std::string("out-messages-") + std::to_string(_current_time) + ".txt";
    std::ofstream f(name);
    LOG(info) << "writing message output to " << name;
    write_stream(f);
  }
  _messages.clear();
  _delays.clear();
}

void message_output::write_stream(std::ostream& out) {
  out << "<state>";
  out << "<delay_and_message_ascii_stream time=\"" << _current_time
      << "\" messages=\"true\">\n";
  out << _messages.size() << " " << _delays.size() << " 0 \n";

  for (message_class const* msg : _messages) {
    msg->write_to_stream(out);
  }

  for (delay_info const* di : _delays) {
    out << di->_schedule_event._train_nr << " "
        << di->_schedule_event._station_index << " "
        << (di->_schedule_event.arrival() ? "a" : "d")
        << unix_time(di->_current_time) << " " << reason_number(di->_reason)
        << " " << unix_time(di->_schedule_event._schedule_time) << " "
        << unix_time(_current_time)  // TODO: release time
        << "\n";
  }

  out << "</delay_and_message_ascii_stream>";
  out << "</state>";
}

}  // namespace realtime
}  // namespace motis
