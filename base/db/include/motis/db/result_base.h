#pragma once

#include <cstdio>

#include "libpq-fe.h"

#include "motis/db/error.h"

namespace motis {
namespace db {

struct result_base {
  result_base(PGresult* res) : res_(res) {}
  virtual ~result_base() {
    PQclear(res_);
  }

  void ensure_valid() {
    ExecStatusType status = PQresultStatus(res_);
    if (status != PGRES_COMMAND_OK && status != PGRES_TUPLES_OK &&
        status != PGRES_SINGLE_TUPLE && status != PGRES_EMPTY_QUERY) {

      // TODO(sebastian) proper logging
      printf("%s: %s\n", PQresStatus(status), PQresultErrorMessage(res_));
      throw std::system_error(exec_status_error::make_error_code(status));
    }
  }

  PGresult* res_;
};

}  // namespace db
}  // namespace motis
