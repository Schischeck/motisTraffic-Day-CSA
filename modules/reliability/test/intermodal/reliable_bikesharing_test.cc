#include "gtest/gtest.h"

#include <iostream>

#include "motis/reliability/tools/flatbuffers/request_builder.h"

#include "../include/test_schedule_setup.h"

namespace motis {
namespace reliability {
namespace intermodal {
namespace bikesharing {

class reliability_bikesharing : public test_motis_setup {
public:
  reliability_bikesharing()
      : test_motis_setup("modules/bikesharing/test_resources/schedule",
                         "20150112", false, true,
                         "modules/bikesharing/test_resources/nextbike") {}
};

TEST_F(reliability_bikesharing, test) {
  // departure close to campus darmstadt
  // arrival close to campus ffm
  auto req_msg = flatbuffers::request_builder::to_bikesharing_request(
      49.8776114, 8.6571044, 50.1273104, 8.6669383,
      1454602500, /* Thu, 04 Feb 2016 16:15:00 GMT */
      1454606100 /* Thu, 04 Feb 2016 17:15:00 GMT */
      );
  auto msg = test::send(motis_instance_, req_msg);
  ASSERT_NE(nullptr, msg);

  std::cout << msg->to_json() << std::endl;
  /*auto response =
      msg->content<::motis::bikesharing::BikesharingResponse const*>();*/
}

}  // namespace bikesharing
}  // namespace intermodal
}  // namespace reliability
}  // namespace motis
