#pragma once

#include <system_error>
#include <type_traits>

namespace motis {
namespace ris {

namespace error {
enum error_code_t { ok = 0, bad_zip_file = 1 };
}  // namespace error

class error_category_impl : public std::error_category {
public:
  const char* name() const noexcept override { return "motis::ris"; }

  std::string message(int ev) const noexcept override {
    switch (ev) {
      case error::ok: return "ris: no error";
      case error::bad_zip_file: return "ris: bad zip file";
      default: return "ris: unkown error";
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

}  // namespace ris
}  // namespace motis

namespace std {

template <>
struct is_error_code_enum<motis::ris::error::error_code_t>
    : public std::true_type {};

}  // namespace std
