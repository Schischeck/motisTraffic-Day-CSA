#pragma once

#include "boost/system/system_error.hpp"
#include "boost/type_traits.hpp"

namespace motis {
namespace ris {

namespace error {
enum error_code_t { ok = 0, not_implemented = 1, unexpected_message = 2 };
}  // namespace error

class error_category_impl : public boost::system::error_category {
public:
  virtual const char* name() const noexcept { return "motis::ris"; }

  virtual std::string message(int ev) const noexcept {
    switch (ev) {
      case error::ok: return "ris: no error";
      case error::not_implemented: return "ris: not implemented";
      case error::unexpected_message: return "ris: unexpected msg (in mode)";
      default: return "ris: unkown error";
    }
  }
};

inline const boost::system::error_category& error_category() {
  static error_category_impl instance;
  return instance;
}

namespace error {
inline boost::system::error_code make_error_code(error_code_t e) noexcept {
  return boost::system::error_code(static_cast<int>(e), error_category());
}
}  // namespace error

}  // namespace ris
}  // namespace motis

namespace boost {
namespace system {

template <>
struct is_error_code_enum<motis::ris::error::error_code_t>
    : public boost::true_type {};

}  // namespace system
}  // namespace boost
