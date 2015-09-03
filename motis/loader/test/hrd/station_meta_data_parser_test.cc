#include <cstring>

#include "boost/filesystem/path.hpp"
#include "gtest/gtest.h"

#include "parser/file.h"

#include "test_spec.h"
#include "motis/loader/util.h"
#include "motis/loader/parser_error.h"
#include "motis/loader/parsers/hrd/files.h"
#include "motis/loader/parsers/hrd/station_meta_data_parser.h"

using namespace parser;
namespace fs = boost::filesystem;

namespace motis {
namespace loader {
namespace hrd {

TEST(station_meta_data_parser, normal_change_times_minimal_file) {
  auto const info_text_file_buf =
      load_file(TEST_RESOURCES / "infotext_minimal.101");
  cstr const info_text_file_content(
      {info_text_file_buf.data(), info_text_file_buf.size()});

  station_meta_data metas;
  parse_station_meta_data({"infotext_minimal.101", info_text_file_content},
                          metas);

  ASSERT_EQ(2, metas.normal_change_times_.size());
  ASSERT_EQ(7, metas.get_interchange_time(8000068));
  ASSERT_EQ(8, metas.get_interchange_time(8000105));
}

TEST(station_meta_data_parser, normal_change_times_mixed_file) {
  auto const info_text_file_buf =
      load_file(TEST_RESOURCES / "infotext_mixed.101");
  cstr const info_text_file_content(
      {info_text_file_buf.data(), info_text_file_buf.size()});

  station_meta_data metas;
  parse_station_meta_data({"infotext_mixed.101", info_text_file_content},
                          metas);

  ASSERT_EQ(2, metas.normal_change_times_.size());
  ASSERT_EQ(7, metas.get_interchange_time(8000068));
  ASSERT_EQ(8, metas.get_interchange_time(8000105));
}

}  // hrd
}  // loader
}  // motis
