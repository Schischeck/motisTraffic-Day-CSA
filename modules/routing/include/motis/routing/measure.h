#pragma once

#include <chrono>

namespace motis {

template <typename time_type = std::chrono::milliseconds>
struct measure {
  template <typename f>
  static typename time_type::rep execution(f const& func) {
    auto start = std::chrono::system_clock::now();
    func();
    auto duration = std::chrono::duration_cast<time_type>(
        std::chrono::system_clock::now() - start);
    return duration.count();
  }
};

}  // namespace motis
