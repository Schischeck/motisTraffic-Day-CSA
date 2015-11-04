#include <cstring>

#include "boost/filesystem/path.hpp"
#include "gtest/gtest.h"

#include "parser/file.h"

#include "test_spec.h"
#include "motis/loader/util.h"
#include "motis/loader/parser_error.h"
#include "motis/loader/parsers/hrd/files.h"
#include "motis/loader/parsers/hrd/station_meta_data_parser.h"

namespace motis {
namespace loader {
namespace hrd {

using namespace parser;
namespace fs = boost::filesystem;

TEST(station_meta_data_parser, normal_change_times_minimal_file) {
  station_meta_data metas;
  loaded_file lf(TEST_RESOURCES / "infotext_minimal.101");
  parse_station_meta_data(lf, metas);

  ASSERT_EQ(2, metas.station_change_times_.size());
  ASSERT_EQ(7, metas.get_station_change_time(8000068));
  ASSERT_EQ(8, metas.get_station_change_time(8000105));
}

TEST(station_meta_data_parser, normal_change_times_mixed_file) {
  station_meta_data metas;
  loaded_file lf(TEST_RESOURCES / "infotext_mixed.101");
  parse_station_meta_data(lf, metas);

  ASSERT_EQ(2, metas.station_change_times_.size());
  ASSERT_EQ(7, metas.get_station_change_time(8000068));
  ASSERT_EQ(8, metas.get_station_change_time(8000105));
}

}  // hrd
}  // loader
}  // motis
