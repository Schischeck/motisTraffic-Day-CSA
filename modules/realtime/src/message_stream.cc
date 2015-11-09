#include "motis/realtime/message_stream.h"

#include <sstream>
#include <fstream>
#include <iomanip>

#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/asio/placeholders.hpp"

#include "motis/core/common/logging.h"
#include "motis/core/common/synchronization.h"
#include "motis/realtime/realtime_schedule.h"
#include "motis/realtime/statistics.h"

namespace motis {
namespace realtime {

using namespace motis::logging;

const char* time_format = "%a %d.%m.%Y (%j) %H:%M";

bool message_stream::start_at(std::time_t start_time) {
  if (start_time >= start_time_) {
    start_time_ = start_time;
    LOG(info) << "Message stream start time set to: " << start_time << " ("
              << std::put_time(std::localtime(&start_time), time_format) << ")";
    return true;
  } else {
    LOG(error) << "Can't set message stream start time to " << start_time
               << " ("
               << std::put_time(std::localtime(&start_time), time_format)
               << "), current start time is " << start_time_ << " ("
               << std::put_time(std::localtime(&start_time_), time_format)
               << ")";
    return false;
  }
}

bool message_stream::forward_to(std::time_t end_time) {
  if (end_time >= end_time_) {
    end_time_ = end_time;
    LOG(info) << "Message stream forwarded to: " << end_time << " ("
              << std::put_time(std::localtime(&end_time), time_format) << ")";
    return true;
  } else {
    LOG(error) << "Can't forward message stream to " << end_time << " ("
               << std::put_time(std::localtime(&end_time), time_format)
               << "), because it is already at " << end_time_ << " ("
               << std::put_time(std::localtime(&end_time_), time_format) << ")";
    return false;
  }
}

std::unique_ptr<message> message_stream::next_message(std::istream& stream) {
  if (cached_msg_ != nullptr) {
    if (cached_msg_->release_time_ < start_time_) {
      cached_msg_.reset(nullptr);
    } else if (cached_msg_->release_time_ > end_time_) {
      return nullptr;
    } else {
      last_msg_time_ = cached_msg_->release_time_;
      return std::move(cached_msg_);
    }
  }
  std::unique_ptr<message> msg;
  do {
    msg = read_next_message(stream);
    if (msg != nullptr && msg->release_time_ >= start_time_) {
      if (msg->release_time_ <= end_time_) {
        last_msg_time_ = msg->release_time_;
        return msg;
      } else {
        cached_msg_ = std::move(msg);
      }
    }
  } while (msg != nullptr);
  return nullptr;
}

std::unique_ptr<message> message_stream::read_next_message(
    std::istream& stream) {
  do {
    if (!stream.good()) {
      fetch();
      continue;
    }
    auto msg = reader_.read_message(stream);
    if (msg != nullptr) {
      return msg;
    }
  } while (stream.good() || !done_);
  return nullptr;
}

//// file_message_stream ////

std::unique_ptr<message> file_message_stream::next_message() {
  return message_stream::next_message(stream_);
}

void file_message_stream::fetch() {
  while (!files_.empty()) {
    std::string name = files_.front();
    files_.erase(files_.begin());
    stream_.close();
    stream_.clear();
    stream_.open(name);
    if (!stream_.is_open() || stream_.fail()) {
      LOG(error) << "Could not open message file " << name;
      continue;
    } else {
      LOG(info) << "Loading messages from: " << name;
      return;
    }
  }
  done_ = true;
}

//// database_message_stream ////
#ifdef WITH_MYSQL

std::unique_ptr<message> database_message_stream::next_message() {
  return message_stream::next_message(stream_);
}

void database_message_stream::fetch() {
  if (db_ == nullptr) {
    return;
  }
  operation_timer timer(rts_._stats._message_fetcher);
  std::time_t from = std::max(start_time_, fetched_until_);
  std::time_t to = std::min(from + max_fetch_size_ * 60, end_time_);
  LOG(info) << "Database: Fetching messages from " << from << " ("
            << std::put_time(std::localtime(&from), time_format) << ") to "
            << to << " (" << std::put_time(std::localtime(&to), time_format)
            << ")...";
  stream_.str(db_->get_messages(from, to));
  stream_.clear();
  fetched_until_ = to;
  done_ = (fetched_until_ == end_time_);
}

bool database_message_stream::forward_to(std::time_t end_time) {
  done_ = false;
  return message_stream::forward_to(end_time);
}

void database_message_stream::set_max_fetch_size(unsigned minutes) {
  max_fetch_size_ = std::max(minutes, 1U);
}

#endif

}  // namespace realtime
}  // namespace motis
