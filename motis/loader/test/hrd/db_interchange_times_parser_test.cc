#include <cstring>

#include "boost/filesystem/path.hpp"

#include "gtest/gtest.h"

#include "parser/file.h"

#include "test_spec.h"
#include "motis/loader/util.h"
#include "motis/loader/parser_error.h"
#include "motis/loader/parsers/hrd/files.h"
#include "motis/loader/parsers/hrd/db_interchange_times_parser.h"

using namespace parser;
namespace fs = boost::filesystem;

namespace motis {
namespace loader {
namespace hrd {

TEST(loader_db_interchange_times, get_interchange_times_minimal) {
  auto const info_text_file_buf =
      load_file(TEST_RESOURCES / "infotext_minimal.101");
  cstr const info_text_file_content(
      {info_text_file_buf.data(), info_text_file_buf.size()});

  db_interchange_times dbit({"infotext_minimal.101", info_text_file_content});

  ASSERT_EQ(2, dbit.eva_num_to_interchange_time_.size());
  ASSERT_EQ(7, dbit.get_interchange_time(8000068));
  ASSERT_EQ(8, dbit.get_interchange_time(8000105));
}

TEST(loader_db_interchange_times, get_interchange_times_mixed) {
  auto const info_text_file_buf =
      load_file(TEST_RESOURCES / "infotext_mixed.101");
  cstr const info_text_file_content(
      {info_text_file_buf.data(), info_text_file_buf.size()});

  db_interchange_times dbit({"infotext_mixed.101", info_text_file_content});

  ASSERT_EQ(2, dbit.eva_num_to_interchange_time_.size());
  ASSERT_EQ(7, dbit.get_interchange_time(8000068));
  ASSERT_EQ(8, dbit.get_interchange_time(8000105));
}

}  // hrd
}  // loader
}  // motis
