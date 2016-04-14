#include "motis/test/schedule/simple_realtime.h"

using namespace flatbuffers;
using namespace motis::module;
using namespace motis::ris;

namespace motis {
namespace test {
namespace schedule {

inline msg_ptr get_simple_realtime_ris_message() {
  FlatBufferBuilder fbb;
  // clang-format off
  std::vector<Offset<UpdatedEvent>> events{
    CreateUpdatedEvent(fbb,
        CreateEvent(fbb,
          fbb.CreateString("8000010"),  // Aschaffenburg
          628,
          fbb.CreateString(""),
          EventType_Departure,
          1448372160  // 2015-11-24 14:36:00 GMT+0100
        ),
      1448372220  // 2015-11-24 14:37:00 GMT+0100
    ),
    CreateUpdatedEvent(fbb,
        CreateEvent(fbb,
          fbb.CreateString("8070003"),  // Frankfurt(M) Flughafen
          628,
          fbb.CreateString(""),
          EventType_Departure,
          1448375100  // 2015-11-24 15:25:00 GMT+0100
        ),
      1448375400  // 2015-11-24 15:30:00 GMT+0100
    )};
  auto trip_id = CreateTripId(fbb,
        fbb.CreateString("8000261"),  // München Hbf
        628,
        1448362440);  // 2015-11-24 11:54:00 GMT+0100
  // clang-format on

  fbb.Finish(CreateMessage(
      fbb, MessageUnion_DelayMessage,
      CreateDelayMessage(fbb, trip_id, DelayType_Is, fbb.CreateVector(events))
          .Union()));

  message_creator mc;
  std::vector<Offset<MessageHolder>> messages{CreateMessageHolder(
      mc, mc.CreateVector(fbb.GetBufferPointer(), fbb.GetSize()))};
  mc.create_and_finish(MsgContent_RISBatch,
                       CreateRISBatch(mc, mc.CreateVector(messages)).Union());
  return make_msg(mc);
}

}  // namespace schedule
}  // namespace test
}  // namespace motis
