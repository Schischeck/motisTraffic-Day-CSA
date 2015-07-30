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

void message_fetcher::load(std::time_t start_time, std::time_t end_time,
                           unsigned interval) {
  const char* time_format = "%a %d.%m.%Y (%j) %H:%M";

  if (end_time == 0) end_time = std::time(nullptr);

  while (start_time < end_time && !_interrupted) {
    std::time_t to = std::min(start_time + interval * 60, end_time);
    LOG(info) << "loading messages from " << start_time << " ("
              << std::put_time(std::localtime(&start_time), time_format)
              << ") to " << to << " ("
              << std::put_time(std::localtime(&to), time_format) << ")...";
    _last_timestamp = to;
    statistics before_stats = _rts._stats;

    std::string messages;
    {
      operation_timer timer(_rts._stats._message_fetcher);
      messages = _db.get_messages(start_time, to);
    }
    {
      std::ofstream f("last-messages.txt");
      f << messages;
    }
    std::istringstream s(messages);
    {
      motis::synchronization::lock(_rts._schedule.sync, true);
      scoped_timer t("process message stream");
      operation_timer timer(_rts._stats._total_processing);
      _rts._message_handler.process_message_stream(s);
    }
    std::cout << "\n";

    statistics run_stats = _rts._stats - before_stats;
    std::cout << "this run:\n";
    run_stats.print(std::cout);
    std::cout << "total so far:\n";
    _rts._stats.print(std::cout);

    {
      std::ofstream f("stats.csv", std::ofstream::app);
      run_stats.write_csv(f, start_time, to);
    }

    start_time = to;
  }
}

void message_fetcher::start_loop() {
  LOG(info) << "starting message fetcher loop";
  boost::posix_time::ptime now =
      boost::posix_time::second_clock::universal_time();
  _timer.expires_at(
      now + boost::posix_time::seconds(60 - now.time_of_day().seconds()));
  _timer.async_wait(boost::bind(&message_fetcher::tick, this,
                                boost::asio::placeholders::error()));
}

void message_fetcher::stop() {
  _interrupted = true;
  _timer.cancel();
}

void message_fetcher::tick(const boost::system::error_code& error) {
  if (error) return;

  if (_last_timestamp == 0) _last_timestamp = std::time(nullptr) - 60;

  load(_last_timestamp);

  _timer.expires_at(_timer.expires_at() + boost::posix_time::minutes(1));
  _timer.async_wait(boost::bind(&message_fetcher::tick, this,
                                boost::asio::placeholders::error()));
}

}  // namespace realtime
}  // namespace motis
