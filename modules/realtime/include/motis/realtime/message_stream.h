#pragma once

#include <ctime>
#include <memory>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>

#include "motis/realtime/messages.h"
#include "motis/realtime/message_reader.h"

#ifdef WITH_MYSQL
#include "Motis/realtime/database.h"
#endif

namespace motis {
namespace realtime {

class realtime_schedule;

class message_stream {
public:
  message_stream(realtime_schedule& rts)
      : rts_(rts),
        reader_(rts),
        done_(false),
        start_time_(0),
        end_time_(0),
        last_msg_time_(0),
        fetched_until_(0) {}

  virtual std::unique_ptr<message> next_message() = 0;
  virtual bool start_at(std::time_t start_time);
  virtual bool forward_to(std::time_t end_time);

  inline std::time_t current_time() const { return last_msg_time_; }
  inline std::time_t start_time() const { return start_time_; }
  inline std::time_t end_time() const { return end_time_; }

protected:
  std::unique_ptr<message> next_message(std::istream& stream);
  std::unique_ptr<message> read_next_message(std::istream& stream);

  virtual void fetch() = 0;

  realtime_schedule& rts_;
  message_reader reader_;
  bool done_;
  std::time_t start_time_;
  std::time_t end_time_;
  std::time_t last_msg_time_;
  std::time_t fetched_until_;
  std::unique_ptr<message> cached_msg_;
};

class file_message_stream : public message_stream {
public:
  file_message_stream(realtime_schedule& rts, std::vector<std::string> files)
      : message_stream(rts), files_(files) {
    done_ = files_.empty();
  }

  std::unique_ptr<message> next_message() override;

protected:
  void fetch() override;

  std::vector<std::string> files_;
  std::ifstream stream_;
};

#ifdef WITH_MYSQL

class database_message_stream : public message_stream {
public:
  database_message_stream(realtime_schedule& rts,
                          std::unique_ptr<delay_database> db)
      : message_stream(rts), db_(std::move(db)), max_fetch_size_(15) {
    done_ = true;
  }

  std::unique_ptr<message> next_message() override;
  bool forward_to(std::time_t end_time) override;
  void set_max_fetch_size(unsigned minutes);

protected:
  void fetch() override;

  std::unique_ptr<delay_database> db_;
  std::istringstream stream_;
  unsigned max_fetch_size_;
};

#endif

}  // namespace realtime
}  // namespace motis
