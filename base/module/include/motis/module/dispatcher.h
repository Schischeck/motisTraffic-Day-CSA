#pragma once

#include "ctx/ctx.h"

#include "motis/module/registry.h"
#include "motis/module/ctx_data.h"
#include "motis/module/future.h"
#include "motis/module/error.h"

namespace motis {
namespace module {

struct dispatcher {
  dispatcher(boost::asio::io_service& ios, registry const& reg)
      : scheduler_(ios), registry_(reg) {}

  future req(msg_ptr const& req, ctx_data const& data, ctx::op_id id) {
    try {
      id.name = req->get()->destination()->target()->str();
      return scheduler_.post(
          data, std::bind(registry_.operations_.at(id.name), req), id);
    } catch (std::out_of_range const&) {
      throw boost::system::system_error(error::target_not_found);
    }
  }

  ctx::scheduler<ctx_data> scheduler_;
  registry const& registry_;
};

}  // namespace module
}  // namespace motis
