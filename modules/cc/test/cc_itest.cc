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

  msg_ptr routing_req_2_11() {
    message_creator fbb;
    fbb.create_and_finish(
        MsgContent_RoutingRequest,
        CreateRoutingRequest(
            fbb, Start_OntripStationStart,
            CreateOntripStationStart(
                fbb, CreateInputStation(fbb, fbb.CreateString("0000002"),
                                        fbb.CreateString("")),
                unix_time(1700))
                .Union(),
            CreateInputStation(fbb, fbb.CreateString("0000011"),
                               fbb.CreateString("")),
            SearchType_SingleCriterion, SearchDir_Forward,
            fbb.CreateVector(std::vector<Offset<Via>>()),
            fbb.CreateVector(std::vector<Offset<AdditionalEdgeWrapper>>()))
            .Union(),
        "/routing");
    return make_msg(fbb);
  }
};

TEST_F(cc_check_routed_connection_test, simple_result_ok) {
  auto res = call(routing_req_2_11());
  std::cout << res->to_json();
}
