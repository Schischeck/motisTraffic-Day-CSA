#pragma once

#include "boost/asio/io_service.hpp"

#include "ctx/ctx.h"

#include "motis/module/dispatcher.h"
#include "motis/module/registry.h"

namespace motis {
namespace module {

struct controller : public dispatcher, public registry {
  controller() : dispatcher(ios_, *this) {}

  template <typename Fn>
  auto run(Fn f) ->
      typename std::enable_if<!std::is_same<void, decltype(f())>::value,
                              decltype(f())>::type {
    decltype(f()) result;
    scheduler_.enqueue(ctx_data(this, sched_), [&]() { result = f(); },
                       ctx::op_id(CTX_LOCATION));
    ios_.run();
    ios_.reset();
    return result;
  }

  template <typename Fn>
  auto run(Fn f) ->
      typename std::enable_if<std::is_same<void, decltype(f())>::value>::type {
    scheduler_.enqueue(ctx_data(this, sched_), [&]() { f(); },
                       ctx::op_id(CTX_LOCATION));
    ios_.run();
    ios_.reset();
  }

  boost::asio::io_service ios_;
};

}  // namespace module
}  // namespace motis
