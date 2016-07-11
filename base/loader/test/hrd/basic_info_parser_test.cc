#include "gtest/gtest.h"

#include "motis/loader/hrd/files.h"
#include "motis/loader/hrd/parser/basic_info_parser.h"

namespace motis {
namespace loader {
namespace hrd {

constexpr auto const ECDATEN_FILE_CONTENT =
    "14.12.2014\n"
    "12.12.2015\n"
    "JF064 EVA_ABN~RIS Server~RIS OEV IMM~~J15~064_001 000000 END\n";

TEST(loader_hrd_basic_info, simple_interval) {
  auto interval = parse_interval({BASIC_DATA_FILE, ECDATEN_FILE_CONTENT});
  EXPECT_EQ(1418515200, interval.from());
  EXPECT_EQ(1449878400, interval.to());
}

TEST(loader_hrd_basic_info, schedule_name) {
  auto name = parse_schedule_name({BASIC_DATA_FILE, ECDATEN_FILE_CONTENT});
  EXPECT_EQ("JF064 EVA_ABN~RIS Server~RIS OEV IMM~~J15~064_001 000000 END",
            name);
}

}  // namespace hrd
}  // namespace loader
}  // namespace motis
