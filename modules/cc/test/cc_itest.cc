#include "gtest/gtest.h"

#include "motis/test/motis_instance_test.h"

#include "./schedule.h"

using namespace flatbuffers;
using namespace motis;
using namespace motis::test;
using namespace motis::module;
using namespace motis::routing;
using motis::cc::dataset_opt;

struct cc_check_routed_connection_test : public motis_instance_test {
  cc_check_routed_connection_test()
      : motis::test::motis_instance_test(dataset_opt, {"cc", "routing", "rt"}) {
  }

  msg_ptr route(std::string const& from, std::string const& to,
                int hhmm) const {
    message_creator fbb;
    fbb.create_and_finish(
        MsgContent_RoutingRequest,
        CreateRoutingRequest(
            fbb, Start_OntripStationStart,
            CreateOntripStationStart(
                fbb, CreateInputStation(fbb, fbb.CreateString(from),
                                        fbb.CreateString("")),
                unix_time(hhmm))
                .Union(),
            CreateInputStation(fbb, fbb.CreateString(to), fbb.CreateString("")),
            SearchType_SingleCriterion, SearchDir_Forward,
            fbb.CreateVector(std::vector<Offset<Via>>()),
            fbb.CreateVector(std::vector<Offset<AdditionalEdgeWrapper>>()))
            .Union(),
        "/routing");
    return make_msg(fbb);
  }

  msg_ptr check_connection(Connection const* con) const {
    message_creator fbb;
    fbb.create_and_finish(MsgContent_Connection,
                          motis_copy_table(Connection, fbb, con).Union(),
                          "/cc");
    return make_msg(fbb);
  }
};

TEST_F(cc_check_routed_connection_test, simple_result_ok) {
  auto const routing_res = call(route("0000002", "0000011", 1700));
  auto const connections =
      motis_content(RoutingResponse, routing_res)->connections();
  ASSERT_EQ(1, connections->size());

  auto const cc_res = call(check_connection(connections->Get(0)));
  EXPECT_NE(MsgContent_MotisError, cc_res->get()->content_type());
}
