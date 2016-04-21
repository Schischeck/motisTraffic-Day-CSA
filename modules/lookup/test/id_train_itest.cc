#include "gtest/gtest.h"

#include "motis/module/message.h"
#include "motis/test/motis_instance_helper.h"
#include "motis/test/schedule/simple_realtime.h"

using namespace motis::test;
using namespace motis::test::schedule;

namespace motis {
namespace lookup {

constexpr auto kIdTrainICERequest = R""(
{ 
  "content_type": "LookupIdTrainRequest",
  "content": { 
    "trip_id": {
      "eva_nr": "8000261", 
      "type": "Departure",
      "train_nr": 628,
      "line_id": "",
      "schedule_time": 1448362440
    }
  }
}
)"";

TEST(lookup, DISABLED_id_train) {
  auto instance = launch_motis(kSimpleRealtimePath, kSimpleRealtimeDate,
                               {"lookup", "realtime"});
  send(instance, get_simple_realtime_ris_message());

  auto msg = send(instance, make_msg(kIdTrainICERequest));
  ASSERT_EQ(MsgContent_LookupIdTrainResponse, msg->content_type());
  auto resp = msg->content<LookupIdTrainResponse const*>();

  auto stops = resp->train()->stops();
  ASSERT_EQ(12, stops->size());

  {
    auto s = stops->Get(0);
    EXPECT_EQ(std::string("8000261"), s->eva_nr()->str());
    EXPECT_EQ(std::string("München Hbf"), s->name()->str());
    EXPECT_DOUBLE_EQ(48.140232, s->lat());
    EXPECT_DOUBLE_EQ(11.558335, s->lng());

    auto a = s->arrival();
    EXPECT_EQ(0, a->schedule_time());
    EXPECT_EQ(0, a->time());
    EXPECT_EQ(std::string(""), a->platform()->str());

    auto d = s->departure();
    EXPECT_EQ(1448362440, d->schedule_time());
    EXPECT_EQ(1448362440, d->time());
    EXPECT_EQ(std::string("23"), d->platform()->str());
  }
  {
    auto s = stops->Get(3);
    EXPECT_EQ(std::string("8000010"), s->eva_nr()->str());
    EXPECT_EQ(std::string("Aschaffenburg Hbf"), s->name()->str());
    EXPECT_DOUBLE_EQ(49.980557, s->lat());
    EXPECT_DOUBLE_EQ(9.143697, s->lng());

    auto a = s->arrival();
    EXPECT_EQ(1448372040, a->schedule_time());
    EXPECT_EQ(1448372040, a->time());
    EXPECT_EQ(std::string("8"), a->platform()->str());

    auto d = s->departure();
    EXPECT_EQ(1448372160, d->schedule_time());
    EXPECT_EQ(1448372220, d->time()); // +1
    EXPECT_EQ(std::string("8"), d->platform()->str());
  }
  {
    auto s = stops->Get(11);
    EXPECT_EQ(std::string("8000080"), s->eva_nr()->str());
    EXPECT_EQ(std::string("Dortmund Hbf"), s->name()->str());
    EXPECT_DOUBLE_EQ(51.517896, s->lat());
    EXPECT_DOUBLE_EQ(7.459290, s->lng());

    auto a = s->arrival();
    EXPECT_EQ(1448382360, a->schedule_time());
    EXPECT_EQ(1448382600, a->time());  // +4 (assuming min standing time = 2)
    EXPECT_EQ(std::string(""), a->platform()->str());  // unknown

    auto d = s->departure();
    EXPECT_EQ(0, d->schedule_time());
    EXPECT_EQ(0, d->time());
    EXPECT_EQ(std::string(""), d->platform()->str());
  }
}

}  // namespace lookup
}  // namespace motis
