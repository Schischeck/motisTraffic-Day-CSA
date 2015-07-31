#ifndef TD_MEASURE_H_
#define TD_MEASURE_H_

#include <chrono>

namespace td {

template<typename TimeT = std::chrono::milliseconds>
struct measure {
  template<typename F>
  static typename TimeT::rep execution(F const &func) {
    auto start = std::chrono::system_clock::now();
    func();
    auto duration = std::chrono::duration_cast<TimeT>(
        std::chrono::system_clock::now() - start);
    return duration.count();
  }
};

}

#endif
