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
  earliest BIGINT NOT NULL,
  latest BIGINT NOT NULL,
  timestamp BIGINT NOT NULL,
  msg BLOB NOT NULL
);
)sql";

constexpr char const* kCreateFullIdxMessage = R"sql(
CREATE INDEX IF NOT EXISTS ris_message_idx ON ris_message (
  timestamp ASC,
  latest DESC,
  earliest ASC
);
)sql";

// clang-format off
SQLPP_DECLARE_TABLE(
  (ris_message),
  (id, int, SQLPP_AUTO_INCREMENT)
  (earliest, bigint, SQLPP_NOT_NULL)
  (latest, bigint, SQLPP_NOT_NULL)
  (timestamp, bigint, SQLPP_NOT_NULL)
  (msg, blob, SQLPP_NOT_NULL)
)
// clang-format on

}  // namespace db

void db_init(db_ptr const& db) {
  db->execute(db::kCreateTabFile);
  db->execute(db::kCreateTabMessage);
  db->execute(db::kCreateFullIdxMessage);
}

std::set<std::string> db_get_files(db_ptr const& db) {
  std::set<std::string> result;

  db::ris_file::ris_file f;
  for (auto const& row : (*db)(select(f.name).from(f).where(true))) {
    result.insert(row.name);
  }

  return result;
}

void db_put_messages(db_ptr const& db, std::string const& filename,
                     std::vector<ris_message> const& msgs) {
  db->start_transaction();

  db::ris_file::ris_file f;
  (*db)(insert_into(f).set(f.name = filename));

  db::ris_message::ris_message m;
  // clang-format off
  auto insert = db->prepare(
      insert_into(m).set(m.earliest = parameter(m.earliest),
                         m.latest = parameter(m.latest),
                         m.timestamp = parameter(m.timestamp),
                         m.msg = parameter(m.msg)));
  // clang-format on
  for (auto const& msg : msgs) {
    insert.params.earliest = msg.earliest;
    insert.params.latest = msg.latest;
    insert.params.timestamp = msg.timestamp;

    std::string b;
    snappy::Compress(reinterpret_cast<char const*>(msg.data()), msg.size(), &b);
    insert.params.msg = {reinterpret_cast<uint8_t const*>(b.data()), b.size()};
    (*db)(insert);
  }

  db->commit_transaction();
}

std::time_t db_get_forward_start_time(db_ptr const& db, std::time_t from,
                                      std::time_t to) {
  db::ris_message::ris_message m;
  auto result = (*db)(select(min(m.timestamp))
                          .from(m)
                          .where(m.latest >= from and m.earliest <= to));

  if (result.empty() || result.front().min.is_null()) {
    return kDBInvalidTimestamp;
  }

  return result.front().min - 1;
}

using blob = std::basic_string<uint8_t>;
std::vector<blob> db_get_messages(db_ptr const& db, std::time_t from,
                                  std::time_t to, std::time_t batch_from,
                                  std::time_t batch_to) {
  std::vector<blob> result;

  db::ris_message::ris_message m;
  for (auto const& row :
       (*db)(select(m.msg)
                 .from(m)
                 .where(m.timestamp > batch_from and m.timestamp <= batch_to and
                        m.latest >= from and m.earliest <= to)
                 .order_by(m.timestamp.asc()))) {

    blob msg = row.msg;
    std::string b;
    snappy::Uncompress(reinterpret_cast<char const*>(msg.data()), msg.size(),
                       &b);
    result.emplace_back(reinterpret_cast<uint8_t const*>(b.data()), b.size());
  }

  return result;
}

void db_clean_messages(db_ptr const& db, std::time_t threshold) {
// TODO 

  db::ris_message::ris_message m;
  (*db)(remove_from(m).where(m.latest < threshold));
}

}  // namespace ris
}  // namespace motis
