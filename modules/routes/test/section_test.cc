#include "gtest/gtest.h"

#include "motis/module/message.h"
#include "motis/test/motis_instance_test.h"
#include "motis/test/schedule/simple_realtime.h"

using namespace motis;
using namespace motis::module;
using namespace motis::test;
using namespace motis::routes;
using motis::test::schedule::simple_realtime::dataset_opt;
using motis::test::schedule::simple_realtime::get_ris_message;

constexpr auto railRoadsSecReqNoRoute = R""(
{
  "destination": {
    "type": "Module",
    "target": "/routes/section"
  },
  "content_type": "RoutesSectionReq",
  "content": {
    "departure": "8000261",
    "arrival": "8000080",
    "clasz": 1
  }
}
)"";

constexpr auto railRoadsSecReq = R""(
{
  "destination": {
    "type": "Module",
    "target": "/routes/section"
  },
  "content_type": "RoutesSectionReq",
  "content": {
    "departure": "8000261",
    "arrival": "8000284",
    "clasz": 0
  }
}
)"";

constexpr auto railRoadsSecReqWrongClasz = R""(
{
  "destination": {
    "type": "Module",
    "target": "/routes/section"
  },
  "content_type": "RoutesSectionReq",
  "content": {
    "departure": "8000261",
    "arrival": "8000284",
    "clasz": 4
  }
}
)"";

const std::vector<std::string> routes_file_param = {
    "--routes.railroads_folder=modules/routes/test/"
    "test_preprocessing/test_osm/simple_realtime.raw"};

struct routes_test : public motis_instance_test {
  routes_test()
      : motis_instance_test(dataset_opt, {"routes"}, routes_file_param) {}
};

TEST_F(routes_test, DISABLED_sec_no_route) {
  auto msg = call(make_msg(railRoadsSecReqNoRoute));
  auto resp = motis_content(RoutesSectionRes, msg);
  EXPECT_EQ(4, resp->section()->size());
  EXPECT_EQ(1, resp->clasz());
  EXPECT_DOUBLE_EQ(48.140232, resp->section()->Get(0));
  EXPECT_DOUBLE_EQ(11.558335, resp->section()->Get(1));
  EXPECT_DOUBLE_EQ(51.517896, resp->section()->Get(2));
  EXPECT_DOUBLE_EQ(7.459290, resp->section()->Get(3));
}

TEST_F(routes_test, DISABLED_sec_with_nodes) {
  auto msg = call(make_msg(railRoadsSecReq));
  auto resp = motis_content(RoutesSectionRes, msg);
  EXPECT_EQ(6, resp->section()->size());
  EXPECT_EQ(0, resp->clasz());
  EXPECT_DOUBLE_EQ(48.140232, resp->section()->Get(0));
  EXPECT_DOUBLE_EQ(11.558335, resp->section()->Get(1));
  EXPECT_DOUBLE_EQ(49.40, resp->section()->Get(2));
  EXPECT_DOUBLE_EQ(11.25, resp->section()->Get(3));
  EXPECT_DOUBLE_EQ(49.445616, resp->section()->Get(4));
  EXPECT_DOUBLE_EQ(11.082989, resp->section()->Get(5));
}

TEST_F(routes_test, DISABLED_sec_wrong_clasz) {
  auto msg = call(make_msg(railRoadsSecReqWrongClasz));
  auto resp = motis_content(RoutesSectionRes, msg);
  EXPECT_EQ(6, resp->section()->size());
  EXPECT_EQ(0, resp->clasz());
  EXPECT_DOUBLE_EQ(48.140232, resp->section()->Get(0));
  EXPECT_DOUBLE_EQ(11.558335, resp->section()->Get(1));
  EXPECT_DOUBLE_EQ(49.40, resp->section()->Get(2));
  EXPECT_DOUBLE_EQ(11.25, resp->section()->Get(3));
  EXPECT_DOUBLE_EQ(49.445616, resp->section()->Get(4));
  EXPECT_DOUBLE_EQ(11.082989, resp->section()->Get(5));
}
