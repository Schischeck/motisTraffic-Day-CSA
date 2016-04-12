#pragma once

#include "ctx/ctx.h"

#include "motis/module/ctx_data.h"
#include "motis/module/dispatcher.h"

namespace motis {
namespace module {

namespace detail {
// taken from: http://stackoverflow.com/a/20441189

// helper class
template <typename R, template <typename...> class Params, typename... Args,
          std::size_t... I>
R call_helper(std::function<R(Args...)> const& func,
              Params<Args...> const& params, std::index_sequence<I...>) {
  return func(std::get<I>(params)...);
}

// "return func(params...)"
template <typename R, template <typename...> class Params, typename... Args>
R call(std::function<R(Args...)> const& func, Params<Args...> const& params) {
  return call_helper(func, params, std::index_sequence_for<Args...>{});
}

}  // namespace detail

template <typename R, typename... Args>
std::function<R(Args...)> motis_wrap(std::function<R(Args...)> fn) {
  auto& op = ctx::current_op<ctx_data>();
  auto& dispatcher = op.data_.dispatcher_;

  ctx::op_id id;
  id.created_at = "motis_wrap";
  id.parent_index = 0;
  id.name = "wrapped_op";

  return [ fn = std::move(fn), &dispatcher, id ](Args... args) {
    auto& scheduler = dispatcher->scheduler_;
    auto& registry = dispatcher->registry_;
    return scheduler.enqueue(
        ctx_data(dispatcher, registry.sched_),
        [ fn = std::move(fn), args = std::make_tuple(std::move(args)...) ] {
          return detail::call(fn, args);
        },
        id);
  };
}

}  // namespace module
}  // namespace motis
