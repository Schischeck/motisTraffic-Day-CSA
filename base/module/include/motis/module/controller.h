#pragma once

#include <thread>

#include "boost/asio/io_service.hpp"

#include "ctx/ctx.h"

#include "motis/module/dispatcher.h"
#include "motis/module/registry.h"
#include "motis/module/run_ios.h"

namespace motis {
namespace module {

struct controller : public dispatcher, public registry {
  controller() : dispatcher(ios_, *this) {}

  template <typename Fn>
  auto run(Fn f, access_t const access = access_t::READ,
           unsigned num_threads = std::thread::hardware_concurrency()) ->
      typename std::enable_if_t<!std::is_same_v<void, decltype(f())>,
                                decltype(f())> {
    decltype(f()) result;
    access == access_t::READ
        ? enqueue_read(ctx_data(access, this, sched_), [&]() { result = f(); },
                       ctx::op_id(CTX_LOCATION))
        : enqueue_write(ctx_data(access, this, sched_), [&]() { result = f(); },
                        ctx::op_id(CTX_LOCATION));
    run_parallel(ios_, num_threads);
    return result;
  }

  template <typename Fn>
  auto run(Fn f, access_t const access = access_t::READ,
           unsigned num_threads = std::thread::hardware_concurrency()) ->
      typename std::enable_if_t<std::is_same_v<void, decltype(f())>> {
    access == access_t::READ
        ? enqueue_read(ctx_data(access, this, sched_), [&]() { f(); },
                       ctx::op_id(CTX_LOCATION))
        : enqueue_write(ctx_data(access, this, sched_), [&]() { f(); },
                        ctx::op_id(CTX_LOCATION));
    run_parallel(ios_, num_threads);
  }

  boost::asio::io_service ios_;
};

}  // namespace module
}  // namespace motis
