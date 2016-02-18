#pragma once

#include "boost/system/system_error.hpp"
#include "boost/type_traits.hpp"

namespace motis {
namespace lookup {

namespace error {
enum error_code_t {
  ok = 0,
  not_implemented = 1,
  station_not_found = 2,
  route_not_found = 3,
  id_event_not_found = 3,
  failure = 127
};
}  // namespace error

class error_category_impl : public boost::system::error_category {
public:
  virtual const char* name() const noexcept { return "motis::lookup"; }

  virtual std::string message(int ev) const noexcept {
    switch (ev) {
      case error::ok: return "lookup: no error";
      case error::not_implemented: return "lookup: not implemented";
      case error::station_not_found: return "lookup: station not found";
      case error::route_not_found: return "lookup: route not found";
      case error::route_not_found: return "lookup: id event not found";
      default: return "lookup: unkown error";
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

}  // namespace lookup
}  // namespace motis

namespace boost {
namespace system {

template <>
struct is_error_code_enum<motis::lookup::error::error_code_t>
    : public boost::true_type {};

}  // namespace system
}  // namespace boost
