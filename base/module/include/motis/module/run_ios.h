#pragma once

#include <stdexcept>

#include "boost/asio/io_service.hpp"

namespace motis {
namespace module {

inline auto run(boost::asio::io_service& ios) {
  return [&ios]() {
    while (true) {
      try {
        ios.run();
        break;
      } catch (std::exception const& e) {
        LOG(motis::logging::emrg) << "unhandled error: " << e.what();
      } catch (...) {
        LOG(motis::logging::emrg) << "unhandled unknown error";
      }
    }
  };
}

inline void run_parallel(
    boost::asio::io_service& ios,
    unsigned num_threads = std::thread::hardware_concurrency()) {
  std::vector<std::thread> threads(num_threads);
  for (auto& t : threads) {
    t = std::thread(run(ios));
  }

  std::for_each(begin(threads), end(threads), [](std::thread& t) { t.join(); });

  ios.reset();
}

}  // namespace module
}  // namespace motis