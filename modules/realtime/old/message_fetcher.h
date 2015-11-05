#pragma once

#include <ctime>
#include <memory>

#include "boost/asio/io_service.hpp"
#include "boost/asio/deadline_timer.hpp"
#include "boost/asio/signal_set.hpp"
#include "boost/bind.hpp"

#include "motis/realtime/message_stream.h"

namespace motis {
namespace realtime {

class realtime_schedule;

class message_fetcher {
public:
  message_fetcher(realtime_schedule& rts,
                  std::unique_ptr<message_stream> msg_stream,
                  boost::asio::io_service& ios)
      : _rts(rts),
        _msg_stream(std::move(msg_stream)),
        _timer(ios),
        _signals(ios, SIGINT, SIGTERM),
        _live(false),
        _interrupted(false) {
    _signals.async_wait(boost::bind(&message_fetcher::set_live, this, false));
  }

  void set_live(bool live);
  void process_stream();

  realtime_schedule& _rts;
  std::unique_ptr<message_stream> _msg_stream;

private:
  void tick(const boost::system::error_code& error);

  boost::asio::deadline_timer _timer;
  boost::asio::signal_set _signals;
  bool _live;
  bool _interrupted;
};

}  // namespace realtime
}  // namespace motis
