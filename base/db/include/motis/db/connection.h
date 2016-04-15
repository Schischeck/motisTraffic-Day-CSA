#pragma once

#include <map>

#include "libpq-fe.h"

#include "motis/db/result_base.h"
#include "motis/db/error.h"

namespace motis {
namespace db {

struct connection_handle {

  connection_handle(std::string conninfo)
      : conn_{PQconnectdb(conninfo.c_str())} {
    if (PQstatus(conn_) != CONNECTION_OK) {
      throw std::system_error(db::error::connection_failed);
    }
  }

  ~connection_handle() { PQfinish(conn_); }

  connection_handle(connection_handle const&) = delete;
  connection_handle& operator=(connection_handle const&) = delete;

  std::string register_prep_stmt(char const* query) {
    auto it = prepared_statements_.find(query);
    if (it == end(prepared_statements_)) {
      auto name =
          prepared_statements_
              .emplace(query, std::to_string(prepared_statements_.size()))
              .first->second;

      result_base r(PQprepare(conn_,  // PGconn *conn
                              name.c_str(),  // const char *stmtName
                              query,  // const char *query,
                              0,  // int nParams,
                              nullptr  // const Oid *paramTypes
                              ));
      r.ensure_valid();

      return name;
    }
    return it->second;
  }

  std::map<std::string, std::string> prepared_statements_;
  PGconn* conn_;
};

}  // namespace db
}  // namespace motis
