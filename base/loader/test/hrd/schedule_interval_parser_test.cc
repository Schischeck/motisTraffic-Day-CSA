#include "gtest/gtest.h"

#include "motis/loader/hrd/parser/schedule_interval_parser.h"
#include "motis/loader/hrd/files.h"

namespace motis {
namespace loader {
namespace hrd {

TEST(loader_hrd_interval, simple_interval) {
  char const* eckdaten_file_content =
      "14.12.2014\n"
      "12.12.2015\n"
      "JF064 EVA_ABN~RIS Server~RIS OEV IMM~~J15~064_001 000000 END\n";
  auto interval = parse_interval({BASIC_DATA_FILE, eckdaten_file_content});
  EXPECT_EQ(1418515200, interval.from());
  EXPECT_EQ(1449878400, interval.to());
}

}  // hrd
}  // loader
}  // motis
