#pragma once

#include <iostream>
#include <iomanip>
#include <numeric>
#include <vector>

namespace motis {
namespace path {

inline void dump_stats(std::vector<size_t>& vec) {
  std::sort(begin(vec), end(vec));
  auto const avg = std::accumulate(begin(vec), end(vec), 0) / vec.size();

  std::sort(begin(vec), end(vec));
  std::cout << " count:" << std::setw(8) << vec.size()  //
            << " avg:" << std::setw(8) << avg  //
            << " q75:" << std::setw(8) << vec[0.75 * (vec.size() - 1)]  //
            << " q90:" << std::setw(8) << vec[0.90 * (vec.size() - 1)]  //
            << " q95:" << std::setw(8) << vec[0.95 * (vec.size() - 1)]  //
            << std::endl;
}

}  // namespace path
}  // namespace motis
