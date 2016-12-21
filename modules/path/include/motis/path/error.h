#pragma once

#include "boost/type_traits.hpp"
#include <system_error>

namespace motis {
namespace path {

namespace error {
enum error_code_t {
  ok = 0,
  database_error = 1,
  not_found = 2,
  unknown_sequence = 3,
};
}  // namespace error

class error_category_impl : public std::error_category {
public:
  const char* name() const noexcept override { return "motis::path"; }

  std::string message(int ev) const noexcept override {
    switch (ev) {
      case error::ok: return "path: no error";
      case error::database_error: return "path: database error";
      case error::not_found: return "path: not found";
      case error::unknown_sequence: return "path: unknown sequence";
      default: return "path: unkown error";
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

}  // namespace path
}  // namespace motis

namespace std {

template <>
struct is_error_code_enum<motis::path::error::error_code_t>
    : public std::true_type {};

}  // namespace std
