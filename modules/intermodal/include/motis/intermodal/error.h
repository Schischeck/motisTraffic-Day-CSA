#pragma once

#include <system_error>
#include <type_traits>

namespace motis {
namespace intermodal {

namespace error {
enum error_code_t { ok = 0, unknown_start = 1, unknown_mode = 2 };
}  // namespace error

class error_category_impl : public std::error_category {
public:
  const char* name() const noexcept override { return "motis::intermodal"; }

  std::string message(int ev) const noexcept override {
    switch (ev) {
      case error::ok: return "intermodal: no error";
      case error::unknown_start: return "intermodal: unknown start type";
      case error::unknown_mode: return "intermodal: unknown mode";
      default: return "intermodal: unkown error";
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

}  // namespace intermodal
}  // namespace motis

namespace std {

template <>
struct is_error_code_enum<motis::intermodal::error::error_code_t>
    : public std::true_type {};

}  // namespace std
