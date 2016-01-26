#pragma once

#include "boost/system/system_error.hpp"
#include "boost/type_traits.hpp"

namespace motis {
namespace bikesharing {

namespace error {
enum error_code_t {
  ok = 0,
  not_implemented = 1,
  database_error = 2,
  terminal_not_found = 3,
};
}  // namespace error

class error_category_impl : public boost::system::error_category {
public:
  virtual const char* name() const noexcept { return "motis::bikesharing"; }

  virtual std::string message(int ev) const noexcept {
    switch (ev) {
      case error::ok: return "bikesharing: no error";
      case error::not_implemented: return "bikesharing: not implemented";
      case error::database_error: return "bikesharing: database error";
      case error::terminal_not_found: return "bikesharing: terminal not found";
      default: return "bikesharing: unkown error";
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

}  // namespace bikesharing
}  // namespace motis

namespace boost {
namespace system {

template <>
struct is_error_code_enum<motis::bikesharing::error::error_code_t>
    : public boost::true_type {};

}  // namespace system
}  // namespace boost
