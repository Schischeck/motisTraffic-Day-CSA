#include <iostream>

#include "gtest/gtest.h"

#include "motis/routes/preprocessing/postgres_writer.h"

using namespace motis;
using namespace motis::routes;

TEST(routes_preprocessing_writer_test, make_line) {
  postgres_writer writer("railways");
  std::string expected =
      "ST_GeomFromText('LINESTRING(0.000000 2.000000, 3.100000 "
      "4.150000)',4269)";
  std::string actual = writer.make_line({0.000, 2.000, 3.100, 4.1500});
  ASSERT_EQ(expected, actual);
}

TEST(routes_preprocessing_writer_test, make_point) {
  postgres_writer writer("railways");
  std::string expected = "ST_SetSRID(ST_MakePoint(0.000000,2.150000), 4326)";
  std::string actual = writer.make_point(0.000, 2.150);
  ASSERT_EQ(expected, actual);
}
