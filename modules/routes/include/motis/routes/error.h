#pragma once

#include "boost/type_traits.hpp"
#include <system_error>

namespace motis {
namespace routes {

namespace error {
enum error_code_t {
  ok = 0,
  database_error = 1,
  not_found = 2,
};
}  // namespace error

class error_category_impl : public std::error_category {
public:
  const char* name() const noexcept override { return "motis::routes"; }

  std::string message(int ev) const noexcept override {
    switch (ev) {
      case error::ok: return "routes: no error";
      case error::database_error: return "routes: database error";
      case error::not_found: return "routes: not found";
      default: return "routes: unkown error";
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

}  // namespace routes
}  // namespace motis

namespace std {

template <>
struct is_error_code_enum<motis::routes::error::error_code_t>
    : public std::true_type {};

}  // namespace std
