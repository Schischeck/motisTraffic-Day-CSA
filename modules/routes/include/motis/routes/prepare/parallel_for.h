#pragma once

#include <atomic>
#include <thread>

#include "motis/core/common/logging.h"

namespace motis {
namespace routes {

template <typename T, typename Fun>
inline void parallel_for(std::string const& desc, std::vector<T> const& jobs,
                         size_t const mod, Fun func) {
  std::atomic<size_t> counter(0);
  std::vector<std::thread> threads;
  for (auto i = 0u; i < std::thread::hardware_concurrency(); ++i) {
    threads.emplace_back([&]() {
      while (true) {
        auto const idx = counter.fetch_add(1);
        if (idx >= jobs.size()) {
          break;
        }

        if (idx % mod == 0) {
          LOG(motis::logging::info) << desc << " " << idx << "/" << jobs.size();
        }

        func(jobs[idx]);
      }
    });
  }

  std::for_each(begin(threads), end(threads), [](auto& t) { t.join(); });
}

}  // namespace routes
}  // namespace motis
