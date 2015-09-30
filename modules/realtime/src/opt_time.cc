#include "motis/realtime/opt_time.h"

std::istream& operator>>(std::istream& in, opt_time& t) {
  std::string s;
  in >> s;

  try {
    t.ptime_ = boost::posix_time::from_iso_string(s);
  } catch (...) {
    try {
      auto date = boost::gregorian::from_undelimited_string(s);
      t.ptime_ = boost::posix_time::ptime(date, boost::posix_time::seconds(0));
    } catch (...) {
      try {
        std::time_t time = boost::lexical_cast<std::time_t>(s);
        t.ptime_ = boost::posix_time::from_time_t(time);
        t.ptime_ = boost::date_time::c_local_adjustor<
            boost::posix_time::ptime>::utc_to_local(t.ptime_);
        std::cout << "--" << t.ptime_ << "--" << std::endl;
      } catch (...) {
        t.ptime_ = boost::posix_time::not_a_date_time;
        std::cerr << "Invalid time: " << s << std::endl;
      }
    }
  }

  return in;
}
