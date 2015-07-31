#ifndef TD_LOGGING_H_
#define TD_LOGGING_H_

#include <ctime>
#include <cstring>
#include <chrono>

#define FILE_NAME (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define LOG(lvl) \
  std::cerr << "\n" \
            << "[" << td::logging::str[lvl] << "]" \
            << "[" << td::logging::time() << "]" \
            << "[" << FILE_NAME << ":" << __LINE__ << "]" \
            << " "

namespace td {
namespace logging {

enum LogLevel { emrg, alrt, crit, error, warn, notice, info, debug };

static const char* const str[] {"emrg", "alrt", "crit", "erro",
                                "warn", "note", "info", "debg"};

inline std::string time()
{
  time_t now;
  std::time(&now);
  char buf[sizeof "2011-10-08T07:07:09Z-0430"];
  strftime(buf, sizeof buf, "%FT%TZ%z", gmtime(&now));
  return buf;
}

struct ScopedTimer final {
  ScopedTimer(char const* name)
      : _name(name),
        _start(std::chrono::steady_clock::now())
  { LOG(info) << "[" << _name << "] starting"; }

  ~ScopedTimer()
  {
    using namespace std::chrono;
    auto stop = steady_clock::now();
    double t = duration_cast<microseconds>(stop - _start).count() / 1000.0;
    LOG(info) << "[" << _name << "] finished" << " (" << t << "ms)";
  }

  const char* _name;
  std::chrono::time_point<std::chrono::steady_clock> _start;
};

}  // namespace logging
}  // namespace td

#endif  // TD_LOGGING_H_