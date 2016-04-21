#include "gtest/gtest.h"

#include <limits>
#include <random>
#include <string>

#include "sqlite3.h"
#include "sqlpp11/sqlpp11.h"
#include "sqlpp11/ppgen.h"
#include "sqlpp11/sqlite3/sqlite3.h"

#include "include/helper.h"

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

db_ptr test_db() {
  sql::connection_config conf;
  conf.path_to_database = ":memory:";
  conf.flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
  conf.debug = false;

  db_ptr db(new sql::connection(conf));
  db_init(db);
  return db;
}

TEST(ris_database, simple_select) {
  auto db = test_db();

  std::vector<ris_message> m;
  m.emplace_back(10, 12, 10, "no_payload");
  db_put_messages(db, "foo", m);

  auto files = db_get_files(db);
  ASSERT_EQ(1, files.size());
  EXPECT_STREQ("foo", begin(files)->c_str());

  // nothing matches
  EXPECT_EQ(0, db_get_messages(db, 0, 9, 0, 9).size());
  EXPECT_EQ(0, db_get_messages(db, 13, 99, 10, 99).size());

  // schedule matches, batch matches
  EXPECT_EQ(1, db_get_messages(db, 0, 10, 0, 99).size());
  EXPECT_EQ(1, db_get_messages(db, 12, 99, 0, 99).size());

  // not in schedule, batch matches
  EXPECT_EQ(0, db_get_messages(db, 0, 9, 0, 99).size());
  EXPECT_EQ(0, db_get_messages(db, 13, 99, 0, 99).size());

  // schedule matches, batch varies
  EXPECT_EQ(0, db_get_messages(db, 0, 99, 0, 9).size());
  EXPECT_EQ(1, db_get_messages(db, 0, 99, 0, 10).size());
  EXPECT_EQ(0, db_get_messages(db, 0, 99, 10, 99).size());
}

TEST(ris_database, sorted) {
  auto db = test_db();
  ris_database_util util;

  util.add_entry(20, 20, 13, "thirteen")
      .add_entry(20, 20, 12, "twelve")
      .finish_packet(db);

  auto result = db_get_messages(db, 0, 99, 0, 99);
  ASSERT_EQ(2, result.size());
  EXPECT_STREQ("twelve", blob_to_cstr(result[0]));
  EXPECT_STREQ("thirteen", blob_to_cstr(result[1]));

  util.add_entry(25, 30, 8, "eight")
      .add_entry(20, 20, 5, "five")
      .finish_packet(db);

  auto result2 = db_get_messages(db, 0, 99, 0, 99);
  ASSERT_EQ(2, result.size());
  EXPECT_STREQ("five", blob_to_cstr(result2[0]));
  EXPECT_STREQ("eight", blob_to_cstr(result2[1]));
  EXPECT_STREQ("twelve", blob_to_cstr(result2[2]));
  EXPECT_STREQ("thirteen", blob_to_cstr(result2[3]));
}

TEST(ris_database, forward) {
  auto db = test_db();
  ris_database_util util;

  util.add_entry(10, 12, 10, "first")
      .add_entry(13, 18, 11, "second")
      .add_entry(16, 20, 12, "third")
      .finish_packet(db);

  EXPECT_EQ(kDBInvalidTimestamp, db_get_forward_start_time(db, 0, 9));  // none
  EXPECT_EQ(9, db_get_forward_start_time(db, 0, 10));  // first
  EXPECT_EQ(9, db_get_forward_start_time(db, 0, 50));  // first
  EXPECT_EQ(10, db_get_forward_start_time(db, 13, 50));  // second
  EXPECT_EQ(10, db_get_forward_start_time(db, 18, 50));  // second
  EXPECT_EQ(11, db_get_forward_start_time(db, 19, 50));  // third
  EXPECT_EQ(kDBInvalidTimestamp,
            db_get_forward_start_time(db, 21, 50));  // none
}

// TODO
// TEST(ris_database, cleanup) {
//   auto db = test_db();

//   std::vector<ris_message> m;
//   m.emplace_back(8, 8, 10, 1, std::unique_ptr<uint8_t>{new uint8_t(1)});
//   db_put_messages("foo", m, db);

//   db_clean_messages(9, db);

//   auto files = db_get_files(db);
//   ASSERT_EQ(1, files.size());
//   EXPECT_EQ(std::string("foo"), *begin(files));

//   auto found_result = db_get_messages(0, 99, 0, 99, db);
//   ASSERT_EQ(0, found_result.size());
// }

}  // namespace ris
}  // namespace motis
