#include "gtest/gtest.h"

#include <iostream>

#include "motis/test/motis_instance_helper.h"
#include "motis/module/message.h"

using namespace motis::test;
using namespace motis::module;

namespace motis {
namespace bikesharing {

constexpr auto kBikesharingRequest = R""(
{
  "content_type": "BikesharingRequest",
  "content": {
    // close to campus darmstadt
    "departure_lat": 49.8776114,
    "departure_lng": 8.6571044,
    
    // close to campus ffm
    "arrival_lat": 50.1273104,
    "arrival_lng": 8.6669383,

    "window_begin": 1454602500,  // Thu, 04 Feb 2016 16:15:00 GMT
    "window_end": 1454606100,  // Thu, 04 Feb 2016 17:15:00 GMT

    "availability_aggregator": "Average"
  }
}
)"";

// nextbike-1454603400.xml -> Thu, 04 Feb 2016 16:30:00 GMT
// nextbike-1454603700.xml -> Thu, 04 Feb 2016 16:35:00 GMT

template <typename V>
BikesharingEdge const* find_edge(V const* vec, std::string const& from,
                                 std::string const& to) {
  for (auto e : *vec) {
    if (e->from()->id()->str() == from && e->to()->id()->str() == to) {
      return e;
    }
  }
  return nullptr;
}

TEST(bikesharing_nextbike_itest, integration_test) {
  auto instance = launch_motis("modules/bikesharing/test_resources/schedule",
                               "20150112", {"bikesharing", "lookup"},
                               {"--bikesharing.nextbike_path=modules/"
                                "bikesharing/test_resources/nextbike",
                                "--bikesharing.database_path=:memory:"});

  auto msg = send(instance, make_msg(kBikesharingRequest));

  ASSERT_EQ(MsgContent_BikesharingResponse, msg->content_type());
  auto resp = msg->content<BikesharingResponse const*>();

  /****************************************************************************
   *  check departure side
   ****************************************************************************/
  {
    auto dep_edges = resp->departure_edges();
    ASSERT_EQ(4, dep_edges->size());

    auto e_1_3 = find_edge(dep_edges, "1", "3");
    ASSERT_NE(nullptr, e_1_3);
    EXPECT_EQ(std::string("8000068"), e_1_3->eva_nr()->str());
    ASSERT_EQ(2, e_1_3->availability()->size());

    auto e_1_a0 = e_1_3->availability()->Get(0);
    EXPECT_EQ(1454601600, e_1_a0->begin());  // Thu, 04 Feb 2016 16:00:00 GMT
    EXPECT_EQ(1454605200, e_1_a0->end());  // Thu, 04 Feb 2016 17:00:00 GMT
    EXPECT_EQ(3, e_1_a0->value());

    auto e_1_a1 = e_1_3->availability()->Get(1);
    EXPECT_EQ(1454605200, e_1_a1->begin());  // Thu, 04 Feb 2016 17:00:00 GMT
    EXPECT_EQ(1454608800, e_1_a1->end());  // Thu, 04 Feb 2016 18:00:00 GMT
    EXPECT_EQ(0, e_1_a1->value());

    auto e_1_4 = find_edge(dep_edges, "1", "4");
    ASSERT_NE(nullptr, e_1_4);
    EXPECT_EQ(std::string("8000068"), e_1_4->eva_nr()->str());
    ASSERT_EQ(2, e_1_4->availability()->size());

    auto e_2_3 = find_edge(dep_edges, "2", "3");
    ASSERT_NE(nullptr, e_2_3);
    EXPECT_EQ(std::string("8000068"), e_2_3->eva_nr()->str());
    ASSERT_EQ(2, e_2_3->availability()->size());

    auto e_2_a0 = e_2_3->availability()->Get(0);
    EXPECT_EQ(1454601600, e_2_a0->begin());  // Thu, 04 Feb 2016 16:00:00 GMT
    EXPECT_EQ(1454605200, e_2_a0->end());  // Thu, 04 Feb 2016 17:00:00 GMT
    EXPECT_EQ(4.5, e_2_a0->value());

    auto e_2_a1 = e_2_3->availability()->Get(1);
    EXPECT_EQ(1454605200, e_2_a1->begin());  // Thu, 04 Feb 2016 17:00:00 GMT
    EXPECT_EQ(1454608800, e_2_a1->end());  // Thu, 04 Feb 2016 18:00:00 GMT
    EXPECT_EQ(0, e_2_a1->value());

    auto e_2_4 = find_edge(dep_edges, "2", "4");
    ASSERT_NE(nullptr, e_2_4);
    EXPECT_EQ(std::string("8000068"), e_2_4->eva_nr()->str());
    ASSERT_EQ(2, e_2_4->availability()->size());
  }

  /****************************************************************************
   *  check arrival side
   ****************************************************************************/
  {
    auto arr_edges = resp->arrival_edges();
    ASSERT_EQ(4, arr_edges->size());

    auto e_1_3 = find_edge(arr_edges, "7", "5");
    ASSERT_NE(nullptr, e_1_3);
    EXPECT_EQ(std::string("8000105"), e_1_3->eva_nr()->str());
    ASSERT_EQ(26, e_1_3->availability()->size());

    auto e_1_a0 = e_1_3->availability()->Get(0);
    EXPECT_EQ(1454601600, e_1_a0->begin());  // Thu, 04 Feb 2016 16:00:00 GMT
    EXPECT_EQ(1454605200, e_1_a0->end());  // Thu, 04 Feb 2016 17:00:00 GMT
    EXPECT_EQ(1, e_1_a0->value());

    auto e_1_a1 = e_1_3->availability()->Get(1);
    EXPECT_EQ(1454605200, e_1_a1->begin());  // Thu, 04 Feb 2016 17:00:00 GMT
    EXPECT_EQ(1454608800, e_1_a1->end());  // Thu, 04 Feb 2016 18:00:00 GMT
    EXPECT_EQ(0, e_1_a1->value());

    auto e_1_4 = find_edge(arr_edges, "7", "6");
    ASSERT_NE(nullptr, e_1_4);
    EXPECT_EQ(std::string("8000105"), e_1_4->eva_nr()->str());
    ASSERT_EQ(26, e_1_4->availability()->size());

    auto e_2_3 = find_edge(arr_edges, "8", "5");
    ASSERT_NE(nullptr, e_2_3);
    EXPECT_EQ(std::string("8000105"), e_2_3->eva_nr()->str());
    ASSERT_EQ(26, e_2_3->availability()->size());

    auto e_2_a0 = e_2_3->availability()->Get(0);
    EXPECT_EQ(1454601600, e_2_a0->begin());  // Thu, 04 Feb 2016 16:00:00 GMT
    EXPECT_EQ(1454605200, e_2_a0->end());  // Thu, 04 Feb 2016 17:00:00 GMT
    EXPECT_EQ(1, e_2_a0->value());

    auto e_2_a1 = e_2_3->availability()->Get(1);
    EXPECT_EQ(1454605200, e_2_a1->begin());  // Thu, 04 Feb 2016 17:00:00 GMT
    EXPECT_EQ(1454608800, e_2_a1->end());  // Thu, 04 Feb 2016 18:00:00 GMT
    EXPECT_EQ(0, e_2_a1->value());

    auto e_2_4 = find_edge(arr_edges, "8", "6");
    ASSERT_NE(nullptr, e_2_4);
    EXPECT_EQ(std::string("8000105"), e_2_4->eva_nr()->str());
    ASSERT_EQ(26, e_2_4->availability()->size());
  }
}

}  // namespace bikesharing
}  // namespace motis
