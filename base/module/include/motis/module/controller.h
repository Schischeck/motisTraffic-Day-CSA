#pragma once

#include "boost/asio/io_service.hpp"

#include "ctx/ctx.h"

#include "motis/module/dispatcher.h"
#include "motis/module/registry.h"

namespace motis {
namespace module {

struct controller : public dispatcher, public registry {
  controller(boost::asio::io_service& ios)
      : dispatcher(ios, *this), ios_(ios) {}

  template <typename Fn>
  auto run(Fn f) -> decltype(f()) {
    decltype(f()) result;
    scheduler_.enqueue(ctx_data(this), [&]() { result = f(); },
                       ctx::op_id(CTX_LOCATION));
    ios_.run();
    return result;
  }

  boost::asio::io_service& ios_;
};

}  // namespace module
}  // namespace motis
