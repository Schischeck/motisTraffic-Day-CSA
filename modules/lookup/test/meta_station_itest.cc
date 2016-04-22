#include "gtest/gtest.h"

#include "motis/module/message.h"
#include "motis/test/motis_instance_helper.h"
#include "motis/test/schedule/simple_realtime.h"

using namespace motis::module;
using namespace motis::test;
using namespace motis::test::schedule::simple_realtime;

namespace motis {
namespace lookup {

constexpr auto kEmptyMetaStationRequest = R""(
{ 
  "destination": { "type": "Module", "target": "/lookup/meta_station" },
  "content_type": "LookupMetaStationRequest",
  "content": { "eva_nr": "8000105" }}
)"";

constexpr auto kMetaStationRequest = R""(
{ 
  "destination": { "type": "Module", "target": "/lookup/meta_station" },
  "content_type": "LookupMetaStationRequest",
  "content": { "eva_nr": "8073368" }}
)"";

TEST(lookup, meta_station) {
  auto motis = launch_motis(kSchedulePath, kScheduleDate, {"lookup"});
  {
    auto msg = call(motis, make_msg(kEmptyMetaStationRequest));
    auto resp = motis_content(LookupMetaStationResponse, msg);
    ASSERT_EQ(0, resp->equivalent()->size());
  }
  {
    auto msg = call(motis, make_msg(kMetaStationRequest));
    auto resp = motis_content(LookupMetaStationResponse, msg);
    ASSERT_EQ(2, resp->equivalent()->size());

    auto e0_eva = resp->equivalent()->Get(0)->eva_nr()->str();
    EXPECT_EQ(std::string("8003368"), e0_eva);

    auto e1_eva = resp->equivalent()->Get(1)->eva_nr()->str();
    EXPECT_EQ(std::string("8073368"), e1_eva);
  }
}

}  // namespace lookup
}  // namespace motis
