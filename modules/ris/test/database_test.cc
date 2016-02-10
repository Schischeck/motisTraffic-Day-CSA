#include "gtest/gtest.h"

#include <limits>
#include <random>
#include <string>

#include "sqlite3.h"
#include "sqlpp11/sqlpp11.h"
#include "sqlpp11/ppgen.h"
#include "sqlpp11/sqlite3/sqlite3.h"

#include "motis/ris/database.h"
#include "motis/ris/ris_message.h"

namespace sql = sqlpp::sqlite3;

namespace motis {
namespace ris {

// some exploratory testing
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
}  // namespace db

std::mt19937 rnd(0);
std::uniform_int_distribution<uint8_t> dist(
    std::numeric_limits<uint8_t>::min(), std::numeric_limits<uint8_t>::max());

std::basic_string<uint8_t> random_string(size_t size) {
  std::basic_string<uint8_t> str;
  for (size_t i = 0; i < size; ++i) {
    str += dist(rnd);
  }
  return str;
}

TEST(ris_database, binary_storage) {
  auto str = random_string(512);

  sql::connection_config conf;
  conf.path_to_database = ":memory:";
  conf.flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
  conf.debug = false;

  sql::connection db(conf);
  db.execute(db::kCreateTabTest);

  db::ris_test::ris_test test;
  auto prepared_insert =
      db.prepare(insert_into(test).set(test.binary = parameter(test.binary)));
  prepared_insert.params.binary = str;
  db(prepared_insert);

  db::ris_test::ris_test test2;
  int count = 0;
  for (auto&& row :
       db(select(test2.id, test2.binary).from(test2).where(true))) {
    ASSERT_EQ(1, row.id) << "more than one row?";
    auto actual = row.binary.value();
    ASSERT_EQ(str.size(), actual.size());
    ASSERT_EQ(str, actual);
    ++count;
  }
  ASSERT_EQ(1, count);
}

TEST(ris_database, basic) {
  sql::connection_config conf;
  conf.path_to_database = ":memory:";
  conf.flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
  conf.debug = false;

  db_ptr db(new sql::connection(conf));
  db_init(db);

  std::vector<ris_message> m;
  m.emplace_back(10, 10, 1, std::unique_ptr<uint8_t>{new uint8_t(1)});

  db_put_messages("foo", m, db);

  auto files = db_get_files(db);
  ASSERT_EQ(1, files.size());
  EXPECT_EQ(std::string("foo"), *begin(files));

  auto empty_result = db_get_messages(11, 12, db);
  ASSERT_EQ(0, empty_result.size());

  auto found_result = db_get_messages(9, 10, db);
  ASSERT_EQ(1, found_result.size());

  // ordering 1
  std::vector<ris_message> m2;
  m2.emplace_back(20, 12, 1, std::unique_ptr<uint8_t>{new uint8_t(0)});
  m2.emplace_back(20, 13, 1, std::unique_ptr<uint8_t>{new uint8_t(1)});

  db_put_messages("ordering1", m2, db);

  auto result2 = db_get_messages(15, 20, db);
  ASSERT_EQ(2, result2.size());
  EXPECT_EQ(std::basic_string<uint8_t>{0}, result2[0]);
  EXPECT_EQ(std::basic_string<uint8_t>{1}, result2[1]);

  // ordering 2
  std::vector<ris_message> m3;
  m3.emplace_back(30, 13, 1, std::unique_ptr<uint8_t>{new uint8_t(0)});
  m3.emplace_back(30, 12, 1, std::unique_ptr<uint8_t>{new uint8_t(1)});

  db_put_messages("ordering2", m3, db);

  auto result3 = db_get_messages(22, 30, db);
  ASSERT_EQ(2, result3.size());
  EXPECT_EQ(std::basic_string<uint8_t>{1}, result3[0]);
  EXPECT_EQ(std::basic_string<uint8_t>{0}, result3[1]);

  EXPECT_EQ(9, db_get_earliest_timestamp_after(0, db));
}

TEST(ris_database, cleanup) {
  sql::connection_config conf;
  conf.path_to_database = ":memory:";
  conf.flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
  conf.debug = false;

  db_ptr db(new sql::connection(conf));
  db_init(db);

  std::vector<ris_message> m;
  m.emplace_back(8, 10, 1, std::unique_ptr<uint8_t>{new uint8_t(1)});
  db_put_messages("foo", m, db);

  db_clean_messages(9, db);

  auto files = db_get_files(db);
  ASSERT_EQ(1, files.size());
  EXPECT_EQ(std::string("foo"), *begin(files));

  auto found_result = db_get_messages(0, 99, db);
  ASSERT_EQ(0, found_result.size());
}

}  // namespace ris
}  // namespace motis
