#pragma once

#include "boost/system/system_error.hpp"
#include "boost/type_traits.hpp"

namespace motis {
namespace routing {

namespace error {
enum error_code_t {
  ok = 0,

  no_guess_for_station = 1,
  given_eva_not_available = 2,
  path_length_too_short = 3,
  journey_date_not_in_schedule = 4,
  unsupported_location_path_element = 5
};
}  // namespace error

class error_category_impl : public boost::system::error_category {
public:
  virtual const char* name() const noexcept { return "motis::routing"; }

  virtual std::string message(int ev) const noexcept {
    switch (ev) {
      case error::ok: return "routing: no error";
      case error::no_guess_for_station:
        return "routing: station could not be guessed";
      case error::given_eva_not_available:
        return "routing: given eva number is not available in this schedule";
      case error::path_length_too_short:
        return "routing: path length < 2 not sensible";
      case error::journey_date_not_in_schedule:
        return "routing: journey date not in schedule";
      case error::unsupported_location_path_element:
        return "unsupported location path element";
      default: return "routing: unkown error";
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

}  // namespace routing
}  // namespace motis

namespace boost {
namespace system {

template <>
struct is_error_code_enum<motis::routing::error::error_code_t>
    : public boost::true_type {};

}  // namespace system
}  // namespace boost
