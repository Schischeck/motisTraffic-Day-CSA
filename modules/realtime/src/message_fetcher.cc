#include "motis/realtime/message_fetcher.h"

#include <sstream>
#include <fstream>

#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/asio/placeholders.hpp"

#include "motis/core/common/logging.h"
#include "motis/core/common/synchronization.h"
#include "motis/realtime/realtime_schedule.h"
#include "motis/realtime/statistics.h"

namespace motis {
namespace realtime {

using namespace motis::logging;

void message_fetcher::set_live(bool live) {
  if (_live && !live) {
    _interrupted = true;
    _timer.cancel();
    _live = false;
    LOG(info) << "Switched to manual mode";
  } else if (!_live && live) {
    boost::posix_time::ptime now =
        boost::posix_time::second_clock::universal_time();
    _timer.expires_at(
        now + boost::posix_time::seconds(60 - now.time_of_day().seconds()));
    _timer.async_wait(boost::bind(&message_fetcher::tick, this,
                                  boost::asio::placeholders::error()));
    _live = true;
    LOG(info) << "Switched to live mode";
  }
}

void message_fetcher::process_stream() {
  if (_msg_stream == nullptr) return;
  statistics before_stats = _rts._stats;

  std::time_t start_time =
      std::max(_msg_stream->current_time(), _msg_stream->start_time());
  std::time_t end_time = _msg_stream->end_time();

  {
    motis::synchronization::lock(_rts._schedule.sync, true);
    scoped_timer t("process message stream");
    operation_timer timer(_rts._stats._total_processing);
    _rts._message_handler.process_message_stream(*_msg_stream);
  }
  std::cout << "\n";

  statistics run_stats = _rts._stats - before_stats;
  std::cout << "this run:\n";
  run_stats.print(std::cout);
  std::cout << "total so far:\n";
  _rts._stats.print(std::cout);

  {
    std::ofstream f("stats.csv", std::ofstream::app);
    run_stats.write_csv(f, start_time, end_time);
  }
}

void message_fetcher::tick(const boost::system::error_code& error) {
  if (error || _msg_stream == nullptr) return;

  _msg_stream->forward_to(std::time(nullptr));
  process_stream();

  _timer.expires_at(_timer.expires_at() + boost::posix_time::minutes(1));
  _timer.async_wait(boost::bind(&message_fetcher::tick, this,
                                boost::asio::placeholders::error()));
}

}  // namespace realtime
}  // namespace motis
