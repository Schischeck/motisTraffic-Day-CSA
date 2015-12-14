#pragma once

#include <string>
#include <ctime>
#include <cstring>
#include <chrono>
#include <iostream>

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
  scoped_timer(char const* name)
      : _name(name), _start(std::chrono::steady_clock::now()) {
    LOG(info) << "[" << _name << "] starting";
  }

  ~scoped_timer() {
    using namespace std::chrono;
    auto stop = steady_clock::now();
    double t = duration_cast<microseconds>(stop - _start).count() / 1000.0;
    LOG(info) << "[" << _name << "] finished"
              << " (" << t << "ms)";
  }

  const char* _name;
  std::chrono::time_point<std::chrono::steady_clock> _start;
};

}  // namespace logging
}  // namespace motis
