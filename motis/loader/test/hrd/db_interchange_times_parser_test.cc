#include <cstring>

#include "boost/filesystem/path.hpp"

#include "../../include/motis/loader/parsers/hrd/change_times_parser.h"
#include "gtest/gtest.h"

#include "parser/file.h"

#include "test_spec.h"
#include "motis/loader/util.h"
#include "motis/loader/parser_error.h"
#include "motis/loader/parsers/hrd/files.h"

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

  change_times ct({"infotext_minimal.101", info_text_file_content});

  ASSERT_EQ(2, ct.eva_num_to_interchange_time_.size());
  ASSERT_EQ(7, ct.get_interchange_time(8000068));
  ASSERT_EQ(8, ct.get_interchange_time(8000105));
}

TEST(loader_db_interchange_times, get_interchange_times_mixed) {
  auto const info_text_file_buf =
      load_file(TEST_RESOURCES / "infotext_mixed.101");
  cstr const info_text_file_content(
      {info_text_file_buf.data(), info_text_file_buf.size()});

  change_times ct({"infotext_mixed.101", info_text_file_content});

  ASSERT_EQ(2, ct.eva_num_to_interchange_time_.size());
  ASSERT_EQ(7, ct.get_interchange_time(8000068));
  ASSERT_EQ(8, ct.get_interchange_time(8000105));
}

}  // hrd
}  // loader
}  // motis
