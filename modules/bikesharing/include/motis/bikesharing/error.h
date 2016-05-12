#pragma once

#include "boost/type_traits.hpp"
#include <system_error>

namespace motis {
namespace bikesharing {

namespace error {
enum error_code_t {
  ok = 0,
  not_implemented = 1,
  not_initialized = 2,
  database_error = 3,
  terminal_not_found = 4,
  search_failure = 5,
};
}  // namespace error

class error_category_impl : public std::error_category {
public:
  virtual const char* name() const noexcept { return "motis::bikesharing"; }

  virtual std::string message(int ev) const noexcept {
    switch (ev) {
      case error::ok: return "bikesharing: no error";
      case error::not_implemented: return "bikesharing: not implemented";
      case error::not_initialized: return "bikesharing: not initialized";
      case error::database_error: return "bikesharing: database error";
      case error::terminal_not_found: return "bikesharing: terminal not found";
      case error::search_failure: return "bikesharing: search_failure";
      default: return "bikesharing: unkown error";
    }
  }
};

inline const std::error_category& error_category() {
  static error_category_impl instance;
  return instance;
}

namespace error {
inline std::error_code make_error_code(error_code_t e) noexcept {
  return std::error_code(static_cast<int>(e), error_category());
}
}  // namespace error

}  // namespace bikesharing
}  // namespace motis

namespace std {

template <>
struct is_error_code_enum<motis::bikesharing::error::error_code_t>
    : public std::true_type {};

}  // namespace std
