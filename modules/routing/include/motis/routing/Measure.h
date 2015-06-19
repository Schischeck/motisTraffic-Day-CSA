#ifndef TD_MEASURE_H_
#define TD_MEASURE_H_

#include <chrono>

namespace td {

template<typename time_type = std::chrono::milliseconds>
struct measure {
  template<typename f>
  static typename time_type::rep execution(f const &func) {
    auto start = std::chrono::system_clock::now();
    func();
    auto duration = std::chrono::duration_cast<time_type>(
        std::chrono::system_clock::now() - start);
    return duration.count();
  }
};

}

#endif
