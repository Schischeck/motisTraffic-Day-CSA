#include "gtest/gtest.h"

#include "motis/loader/gtfs/transfers.h"
#include "motis/loader/gtfs/files.h"

#include "./resources.h"

using namespace parser;
using namespace motis::loader;
using namespace motis::loader::gtfs;

stop_pair t(stop_map const& stops, std::string const& s1,
            std::string const& s2) {
  return std::make_pair(stops.at(s1).get(), stops.at(s2).get());
}

TEST(loader_gtfs_transfer, read_transfers_example_data) {
  auto stops = read_stops(loaded_file{SCHEDULES / "example" / STOPS_FILE});
  auto transfers = read_transfers(
      loaded_file{SCHEDULES / "example" / TRANSFERS_FILE}, stops);

  EXPECT_EQ(3, transfers.size());

  EXPECT_EQ(5, transfers[t(stops, "S6", "S7")].minutes_);
  EXPECT_EQ(transfer::MIN_TRANSFER_TIME, transfers[t(stops, "S6", "S7")].type_);
  EXPECT_EQ(transfer::NOT_POSSIBLE, transfers[t(stops, "S7", "S6")].type_);
  EXPECT_EQ(transfer::TIMED_TRANSFER, transfers[t(stops, "S23", "S7")].type_);
}

TEST(loader_gtfs_transfer, read_transfers_berlin_data) {
  auto stops = read_stops(loaded_file{SCHEDULES / "berlin" / STOPS_FILE});
  auto transfers =
      read_transfers(loaded_file{SCHEDULES / "berlin" / TRANSFERS_FILE}, stops);

  EXPECT_EQ(5, transfers.size());

  EXPECT_EQ(3, transfers[t(stops, "9003104", "9003174")].minutes_);
  EXPECT_EQ(transfer::MIN_TRANSFER_TIME,
            transfers[t(stops, "9003104", "9003174")].type_);

  EXPECT_EQ(4, transfers[t(stops, "9003104", "9003175")].minutes_);
  EXPECT_EQ(transfer::MIN_TRANSFER_TIME,
            transfers[t(stops, "9003104", "9003175")].type_);

  EXPECT_EQ(3, transfers[t(stops, "9003104", "9003176")].minutes_);
  EXPECT_EQ(transfer::MIN_TRANSFER_TIME,
            transfers[t(stops, "9003104", "9003176")].type_);

  EXPECT_EQ(3, transfers[t(stops, "9003174", "9003104")].minutes_);
  EXPECT_EQ(transfer::MIN_TRANSFER_TIME,
            transfers[t(stops, "9003174", "9003104")].type_);

  EXPECT_EQ(3, transfers[t(stops, "9003174", "9003175")].minutes_);
  EXPECT_EQ(transfer::MIN_TRANSFER_TIME,
            transfers[t(stops, "9003174", "9003175")].type_);
}
