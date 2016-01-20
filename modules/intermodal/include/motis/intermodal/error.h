#pragma once

#include "boost/system/system_error.hpp"
#include "boost/type_traits.hpp"

namespace motis {
namespace intermodal {

namespace error {
enum error_code_t { ok = 0, not_implemented = 1 };
}  // namespace error

class error_category_impl : public boost::system::error_category {
public:
  virtual const char* name() const noexcept { return "motis::intermodal"; }

  virtual std::string message(int ev) const noexcept {
    switch (ev) {
      case error::ok: return "intermodal: no error";
      case error::not_implemented: return "intermodal: not implemented";
      default: return "intermodal: unkown error";
    }
  }
};

const boost::system::error_category& error_category() {
  static error_category_impl instance;
  return instance;
}

namespace error {
boost::system::error_code make_error_code(error_code_t e) noexcept {
  return boost::system::error_code(static_cast<int>(e), error_category());
}
}  // namespace error

}  // namespace intermodal
}  // namespace motis

namespace boost {
namespace system {

template <>
struct is_error_code_enum<motis::intermodal::error::error_code_t>
    : public boost::true_type {};

}  // namespace system
}  // namespace boost
