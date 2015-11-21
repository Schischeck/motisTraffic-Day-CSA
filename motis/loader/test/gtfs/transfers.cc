#include "gtest/gtest.h"

#include "motis/loader/gtfs/transfers.h"
#include "motis/loader/gtfs/files.h"

using namespace parser;
using namespace motis::loader::gtfs;

TEST(loader_gtfs_transfer, read_transfers_example_data) {
  auto transfers =
      read_transfers({TRANSFERS_FILE, example_transfers_file_content});

  EXPECT_EQ(3, transfers.size());

  EXPECT_EQ(5, transfers[std::make_pair("S6", "S7")].minutes);
  EXPECT_EQ(transfer::MIN_TRANSFER_TIME,
            transfers[std::make_pair("S6", "S7")].type);

  EXPECT_EQ(transfer::NOT_POSSIBLE, transfers[std::make_pair("S7", "S6")].type);

  EXPECT_EQ(transfer::TIMED_TRANSFER,
            transfers[std::make_pair("S23", "S7")].type);
}

TEST(loader_gtfs_transfer, read_transfers_berlin_data) {
  auto transfers =
      read_transfers({TRANSFERS_FILE, berlin_transfers_file_content});

  EXPECT_EQ(5, transfers.size());

  EXPECT_EQ(3, transfers[std::make_pair("9003104", "9003174")].minutes);
  EXPECT_EQ(transfer::MIN_TRANSFER_TIME,
            transfers[std::make_pair("9003104", "9003174")].type);

  EXPECT_EQ(4, transfers[std::make_pair("9003104", "9003175")].minutes);
  EXPECT_EQ(transfer::MIN_TRANSFER_TIME,
            transfers[std::make_pair("9003104", "9003175")].type);

  EXPECT_EQ(3, transfers[std::make_pair("9003104", "9003176")].minutes);
  EXPECT_EQ(transfer::MIN_TRANSFER_TIME,
            transfers[std::make_pair("9003104", "9003176")].type);

  EXPECT_EQ(3, transfers[std::make_pair("9003174", "9003104")].minutes);
  EXPECT_EQ(transfer::MIN_TRANSFER_TIME,
            transfers[std::make_pair("9003174", "9003104")].type);

  EXPECT_EQ(3, transfers[std::make_pair("9003174", "9003175")].minutes);
  EXPECT_EQ(transfer::MIN_TRANSFER_TIME,
            transfers[std::make_pair("9003174", "9003175")].type);
}