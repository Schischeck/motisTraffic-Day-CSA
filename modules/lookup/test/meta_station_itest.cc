#include "gtest/gtest.h"

#include "motis/module/message.h"
#include "motis/test/motis_instance_helper.h"
#include "motis/test/schedule/simple_realtime.h"

using namespace motis::test;
using namespace motis::test::schedule;

namespace motis {
namespace lookup {

constexpr auto kEmptyMetaStationRequest = R""(
{ "content_type": "LookupMetaStationRequest",
  "content": { "eva_nr": "8000105" }}
)"";

constexpr auto kMetaStationRequest = R""(
{ "content_type": "LookupMetaStationRequest",
  "content": { "eva_nr": "8073368" }}
)"";

TEST(lookup, meta_station) {
  auto i = launch_motis(kSimpleRealtimePath, kSimpleRealtimeDate, {"lookup"});
  {
    auto msg = send(i, make_msg(kEmptyMetaStationRequest));
    ASSERT_EQ(MsgContent_LookupMetaStationResponse, msg->content_type());
    auto res = msg->content<LookupMetaStationResponse const*>();
    ASSERT_EQ(0, res->equivalent()->size());
  }
  {
    auto msg = send(i, make_msg(kMetaStationRequest));
    ASSERT_EQ(MsgContent_LookupMetaStationResponse, msg->content_type());
    auto res = msg->content<LookupMetaStationResponse const*>();

    ASSERT_EQ(2, res->equivalent()->size());

    auto e0_eva = res->equivalent()->Get(0)->eva_nr()->str();
    EXPECT_EQ(std::string("8003368"), e0_eva);

    auto e1_eva = res->equivalent()->Get(1)->eva_nr()->str();
    EXPECT_EQ(std::string("8073368"), e1_eva);
  }
}


} // namespace lookup
} // namespace motis
