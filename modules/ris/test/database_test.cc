#include "gtest/gtest.h"

#include <limits>
#include <random>
#include <string>

#include "sqlite3.h"
#include "sqlpp11/sqlpp11.h"
#include "sqlpp11/ppgen.h"
#include "sqlpp11/sqlite3/sqlite3.h"

// some exploratory testing

namespace sql = sqlpp::sqlite3;

namespace motis {
namespace ris {

namespace db {
// clang-format off
constexpr char const* kCreateTabTest = R"sql(
CREATE TABLE IF NOT EXISTS ris_test (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  binary BLOB NOT NULL
);
)sql";

SQLPP_DECLARE_TABLE(
  (ris_test),
  (id, int, SQLPP_AUTO_INCREMENT)
  (binary, blob,SQLPP_NOT_NULL)
)
// clang-format on
} // namespace db

TEST(ris_database, binary_storage) {
  std::mt19937 rnd(0);
  std::uniform_int_distribution<uint8_t> dist(std::numeric_limits<uint8_t>::min(), std::numeric_limits<uint8_t>::max());

  std::basic_string<uint8_t> str;
  for(int i = 0; i < 512; ++i) {
    str += dist(rnd);
  }

  sql::connection_config conf;
  conf.path_to_database = ":memory:";
  conf.flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
  conf.debug = false;

  sql::connection db(conf);
  db.execute(db::kCreateTabTest);

  db::ris_test::ris_test test;
  auto prepared_insert = db.prepare(insert_into(test).set(test.binary = parameter(test.binary)));
  prepared_insert.params.binary = str;
  db(prepared_insert);

  db::ris_test::ris_test test2;
  int count = 0;
  for(auto&& row : db(select(test2.id, test2.binary).from(test2).where(true))) {
    ASSERT_EQ(1, row.id) << "more than one row?";
    auto actual = row.binary.value();
    ASSERT_EQ(str.size(), actual.size());
    ASSERT_EQ(str, actual);
    ++count;
  }
  ASSERT_EQ(1, count);
}

}  // namespace ris
}  // namespace motis
