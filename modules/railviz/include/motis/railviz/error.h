#pragma once

#include "boost/system/system_error.hpp"
#include "boost/type_traits.hpp"

namespace motis {
namespace railviz {

namespace error {
enum error_code_t {
  ok = 0,

  station_index_out_of_bounds = 1,
  client_not_registered = 2,

  route_not_found = 3,
  train_not_found = 4,
};
}  // namespace error

class error_category_impl : public boost::system::error_category {
 public:
  virtual const char* name() const noexcept { return "motis::railviz"; }

  virtual std::string message(int ev) const noexcept {
    switch (ev) {
      case error::station_index_out_of_bounds:
        return "railviz: station index out of bounds";
      case error::client_not_registered:
        return "railviz: client not registered";
      case error::route_not_found:
        return "railviz: route not found";
      case error::train_not_found:
        return "railviz: train not found";
      default:
        return "railviz: unkown error";
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

}  // namespace railviz
}  // namespace motis

namespace boost {
namespace system {

template <>
struct is_error_code_enum<motis::railviz::error::error_code_t>
    : public boost::true_type {};

}  // namespace system
}  // namespace boost
