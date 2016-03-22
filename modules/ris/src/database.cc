#include "motis/ris/database.h"

#include "snappy.h"
#include "sqlite3.h"
#include "sqlpp11/sqlpp11.h"
#include "sqlpp11/ppgen.h"
#include "sqlpp11/sqlite3/sqlite3.h"

#include "motis/ris/ris_message.h"

namespace sql = sqlpp::sqlite3;

namespace motis {
namespace ris {

namespace db {
constexpr char const* kCreateTabFile = R"sql(
CREATE TABLE IF NOT EXISTS ris_file (
  name VARCHAR(255)
);
)sql";

// clang-format off
SQLPP_DECLARE_TABLE(
  (ris_file),
  (id, int, SQLPP_AUTO_INCREMENT)
  (name, varchar(255),SQLPP_NOT_NULL)
)
// clang-format on

constexpr char const* kCreateTabMessage = R"sql(
CREATE TABLE IF NOT EXISTS ris_message (
  scheduled BIGINT NOT NULL,
  timestamp BIGINT NOT NULL,
  msg BLOB NOT NULL
);
)sql";

constexpr char const* kCreateFullIdxMessage = R"sql(
CREATE INDEX IF NOT EXISTS ris_message_idx ON ris_message (
  scheduled ASC,
  timestamp ASC
);
)sql";

constexpr char const* kCreateTimestampIdxMessage = R"sql(
CREATE INDEX IF NOT EXISTS ris_message_timestamp_idx ON ris_message (
  timestamp ASC
);
)sql";

// clang-format off
SQLPP_DECLARE_TABLE(
  (ris_message),
  (id, int, SQLPP_AUTO_INCREMENT)
  (scheduled, bigint, SQLPP_NOT_NULL)
  (timestamp, bigint, SQLPP_NOT_NULL)
  (msg, blob, SQLPP_NOT_NULL)
)
// clang-format on

}  // namespace db

db_ptr default_db() {
  sql::connection_config conf;
  conf.path_to_database = "ris.sqlite3";
  conf.flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
  conf.debug = false;
  return db_ptr(new sql::connection(conf));
}

void db_init(db_ptr const& db) {
  db->execute(db::kCreateTabFile);
  db->execute(db::kCreateTabMessage);
  db->execute(db::kCreateFullIdxMessage);
  db->execute(db::kCreateTimestampIdxMessage);
}

std::set<std::string> db_get_files(db_ptr const& db) {
  std::set<std::string> result;

  db::ris_file::ris_file f;
  for (auto const& row : (*db)(select(f.name).from(f).where(true))) {
    result.insert(row.name);
  }

  return result;
}

void db_put_messages(std::string const& filename,
                     std::vector<ris_message> const& msgs, db_ptr const& db) {
  db->start_transaction();

  db::ris_file::ris_file f;
  (*db)(insert_into(f).set(f.name = filename));

  db::ris_message::ris_message m;
  // clang-format off
  auto insert = db->prepare(
      insert_into(m).set(m.scheduled = parameter(m.scheduled),
                         m.timestamp = parameter(m.timestamp),
                         m.msg = parameter(m.msg)));
  // clang-format on
  for (auto const& msg : msgs) {
    insert.params.scheduled = msg.scheduled;
    insert.params.timestamp = msg.timestamp;

    std::string b;
    snappy::Compress(reinterpret_cast<char const*>(msg.data()), msg.size(), &b);
    insert.params.msg = {reinterpret_cast<uint8_t const*>(b.data()), b.size()};
    (*db)(insert);
  }

  db->commit_transaction();
}

std::time_t db_get_forward_start_time(db_ptr const& db) {
  db::ris_message::ris_message m;
  auto result = (*db)(select(min(m.timestamp)).from(m).where(true));

  if (result.empty() || result.front().min.is_null()) {
    return kDBInvalidTimestamp;
  }

  return result.front().min - 1;
}

std::vector<std::basic_string<uint8_t>> db_get_messages(std::time_t from,
                                                        std::time_t to,
                                                        db_ptr const& db) {
  std::vector<std::basic_string<uint8_t>> result;

  db::ris_message::ris_message m;
  for (auto const& row :
       (*db)(select(m.msg)
                 .from(m)
                 .where(m.scheduled > from and m.scheduled <= to)
                 .order_by(m.timestamp.asc()))) {

    std::basic_string<uint8_t> msg = row.msg;
    std::string b;
    snappy::Uncompress(reinterpret_cast<char const*>(msg.data()), msg.size(),
                       &b);
    result.emplace_back(reinterpret_cast<uint8_t const*>(b.data()), b.size());
  }

  return result;
}

void db_clean_messages(std::time_t threshold, db_ptr const& db) {
  db::ris_message::ris_message m;
  (*db)(remove_from(m).where(m.scheduled < threshold));
}

}  // namespace ris
}  // namespace motis
