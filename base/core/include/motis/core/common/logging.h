#pragma once

#include <chrono>
#include <cstring>
#include <ctime>
#include <iostream>
#include <string>

#define FILE_NAME \
  (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define LOG(lvl)                                          \
  std::cerr << "\n"                                       \
            << "[" << motis::logging::str[lvl] << "]"     \
            << "[" << motis::logging::time() << "]"       \
            << "[" << FILE_NAME << ":" << __LINE__ << "]" \
            << " "

namespace motis {
namespace logging {

enum log_level { emrg, alrt, crit, error, warn, notice, info, debug };

static const char* const str[]{"emrg", "alrt", "crit", "erro",
                               "warn", "note", "info", "debg"};

inline std::string time() {
  time_t now;
  std::time(&now);
  char buf[sizeof "2011-10-08t07:07:09z-0430"];
  strftime(buf, sizeof buf, "%FT%TZ%z", gmtime(&now));
  return buf;
}

struct scoped_timer final {
  explicit scoped_timer(char const* name)
      : name_(name), start_(std::chrono::steady_clock::now()) {
    LOG(info) << "[" << name_ << "] starting";
  }

  ~scoped_timer() {
    using namespace std::chrono;
    auto stop = steady_clock::now();
    double t = duration_cast<microseconds>(stop - start_).count() / 1000.0;
    LOG(info) << "[" << name_ << "] finished"
              << " (" << t << "ms)";
  }

  const char* name_;
  std::chrono::time_point<std::chrono::steady_clock> start_;
};

struct manual_timer final {
  explicit manual_timer(std::string name)
      : name_(std::move(name)), start_(std::chrono::steady_clock::now()) {
    LOG(info) << "[" << name_ << "] starting";
  }

  void stop_and_print() {
    using namespace std::chrono;
    auto stop = steady_clock::now();
    double t = duration_cast<microseconds>(stop - start_).count() / 1000.0;
    LOG(info) << "[" << name_ << "] finished"
              << " (" << t << "ms)";
  }

  std::string name_;
  std::chrono::time_point<std::chrono::steady_clock> start_;
};

}  // namespace logging
}  // namespace motis
