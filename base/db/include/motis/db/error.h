#pragma once

#include <system_error>
#include <type_traits>

namespace motis {
namespace db {

namespace error {
enum error_code_t {
  ok = 0,
  connection_failed = 1,
  command_failed = 2,
};
}  // namespace error

class error_category_impl : public std::error_category {
public:
  const char* name() const noexcept override { return "motis::db"; }

  std::string message(int ev) const noexcept override {
    switch (ev) {
      case error::ok: return "db: no error";
      case error::connection_failed: return "db: connection failed";
      case error::command_failed: return "db: command failed";
      default: return "db: unkown error";
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

namespace exec_status_error {
class error_category_impl : public std::error_category {
public:
  const char* name() const noexcept override {
    return "postgres::ExecStatusType";
  }

  std::string message(int ev) const noexcept override {
    return PQresStatus(static_cast<ExecStatusType>(ev));
  }
};

inline const std::error_category& error_category() {
  static error_category_impl instance;
  return instance;
}

inline std::error_code make_error_code(ExecStatusType e) noexcept {
  return std::error_code(static_cast<int>(e), error_category());
}
}  // namespace exec_status_error

}  // namespace db
}  // namespace motis

namespace std {

template <>
struct is_error_code_enum<motis::db::error::error_code_t>
    : public std::true_type {};

template <>
struct is_error_code_enum<ExecStatusType> : public std::true_type {};

}  // namespace std
