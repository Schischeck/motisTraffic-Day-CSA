#include "motis/ris/database.h"

#include <iostream>

#include "sqlite3.h"
#include "sqlpp11/sqlpp11.h"
#include "sqlpp11/ppgen.h"
#include "sqlpp11/sqlite3/sqlite3.h"

namespace sql = sqlpp::sqlite3;

namespace motis {
namespace ris {

namespace db {
// clang-format off
constexpr char const* kCreateTabFile = R"sql(
CREATE TABLE IF NOT EXISTS ris_file (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  name VARCHAR(255)
);
)sql";

SQLPP_DECLARE_TABLE(
  (ris_file),
  (id, int, SQLPP_AUTO_INCREMENT)
  (name, varchar(255),SQLPP_NOT_NULL)
)

constexpr char const* kCreateTabMessage = R"sql(
CREATE TABLE IF NOT EXISTS ris_message (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  scheduled BIGINT NOT NULL,
  timestamp BIGINT NOT NULL,
  msg BLOB NOT NULL
);
)sql";

SQLPP_DECLARE_TABLE(
  (ris_message),
  (id, int, SQLPP_AUTO_INCREMENT)
  (scheduled, bigint, SQLPP_NOT_NULL)
  (timestamp, bigint, SQLPP_NOT_NULL)
  (msg, blob, SQLPP_NOT_NULL)
)
// clang-format on
}  // namespace db

sql::connection_config config() {
  sql::connection_config conf;
  conf.path_to_database = "ris.sqlite3";
  conf.flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
  conf.debug = false;
  return conf;
}

void db_init() {
  sql::connection db(config());
  db.execute(db::kCreateTabFile);
  db.execute(db::kCreateTabMessage);
}

std::set<std::string> db_get_stored_files() {
  std::set<std::string> result;

  sql::connection db(config());
  db::ris_file::ris_file f;
  for (auto const& row : db(select(f.name).from(f).where(true))) {
    result.insert(row.name);
  }

  return result;
}

void db_put_messages(std::string const& filename,
                     std::vector<ris_message> const& msgs) {
  sql::connection db(config());
  db.start_transaction();

  db::ris_file::ris_file f;
  db(insert_into(f).set(f.name = filename));

  db::ris_message::ris_message m;
  auto insert = db.prepare(insert_into(m).set(
      m.scheduled = parameter(m.scheduled),
      m.timestamp = parameter(m.timestamp), m.msg = parameter(m.msg)));
  for (auto const& msg : msgs) {
    insert.params.scheduled = msg.scheduled;
    insert.params.timestamp = msg.timestamp;
    insert.params.msg = {msg.data(), msg.size()};
    db(insert);
  }

  db.commit_transaction();
}

std::vector<std::basic_string<uint8_t>> db_get_messages(std::time_t /*from*/,
                                                        std::time_t /*to*/) {
  // TODO
  
  return {};
}

}  // namespace ris
}  // namespace motis
