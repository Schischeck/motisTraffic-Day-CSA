#pragma once

#include "boost/system/system_error.hpp"
#include "boost/type_traits.hpp"

namespace motis {
namespace module {

namespace error {
enum error_code_t {
  ok = 0,

  no_module_capable_of_handling = 1,
  unable_to_parse_msg = 2
};
}  // namespace error

class error_category_impl : public boost::system::error_category {
public:
  virtual const char* name() const noexcept { return "motis::module"; }

  virtual std::string message(int ev) const noexcept {
    switch (ev) {
      case error::ok:
        return "module: no error";
      case error::no_module_capable_of_handling:
        return "module: there is no module capable of handling the message";
      case error::unable_to_parse_msg:
        return "module: unable to parse message";
      default:
        return "module: unkown error";
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

}  // namespace module
}  // namespace motis

namespace boost {
namespace system {

template <>
struct is_error_code_enum<motis::module::error::error_code_t>
    : public boost::true_type {};

}  // namespace system
}  // namespace boost
