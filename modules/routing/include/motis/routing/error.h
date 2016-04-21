#pragma once

#include <system_error>
#include <type_traits>

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

class error_category_impl : public std::error_category {
public:
  const char* name() const noexcept override { return "motis::routing"; }

  std::string message(int ev) const noexcept override {
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

inline const std::error_category& error_category() {
  static error_category_impl instance;
  return instance;
}

namespace error {
inline std::error_code make_error_code(error_code_t e) noexcept {
  return std::error_code(static_cast<int>(e), error_category());
}
}  // namespace error

}  // namespace routing
}  // namespace motis

namespace std {

template <>
struct is_error_code_enum<motis::routing::error::error_code_t>
    : public std::true_type {};

}  // namespace std
