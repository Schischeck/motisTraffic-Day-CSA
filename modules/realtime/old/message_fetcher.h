#pragma once

#include <ctime>

#include "boost/asio/io_service.hpp"
#include "boost/asio/deadline_timer.hpp"
#include "boost/asio/signal_set.hpp"
#include "boost/bind.hpp"

#include "motis/realtime/database.h"

namespace motis {
namespace realtime {

class realtime_schedule;

class message_fetcher {
public:
  message_fetcher(realtime_schedule& rts, delay_database& db,
                  boost::asio::io_service& ios)
      : _rts(rts),
        _db(db),
        _timer(ios),
        _signals(ios, SIGINT, SIGTERM),
        _last_timestamp(0),
        _interrupted(false) {
    _signals.async_wait(boost::bind(&message_fetcher::stop, this));
  }

  void load(std::time_t start_time, std::time_t end_time = 0,
            unsigned interval = 1);
  void start_loop();
  void stop();

private:
  void tick(const boost::system::error_code& error);

  realtime_schedule& _rts;
  delay_database& _db;
  boost::asio::deadline_timer _timer;
  boost::asio::signal_set _signals;
  std::time_t _last_timestamp;
  bool _interrupted;
};

}  // namespace realtime
}  // namespace motis
