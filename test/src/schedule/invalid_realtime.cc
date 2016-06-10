#include "motis/test/schedule/invalid_realtime.h"

#include "motis/core/schedule/schedule.h"
#include "motis/core/access/time_access.h"

using namespace flatbuffers;
using namespace motis::module;
using namespace motis::ris;

namespace motis {
namespace test {
namespace schedule {
namespace invalid_realtime {

void create_is_msg(motis::schedule const& sched, FlatBufferBuilder& fbb) {
  // clang-format off
  std::vector<Offset<UpdatedEvent>> events{
    CreateUpdatedEvent(fbb,
        motis::ris::CreateEvent(fbb,
          fbb.CreateString("0000002"),
          1,
          fbb.CreateString("381"),
          motis::ris::EventType_Arrival,
          unix_time(sched, 1100)
        ),
        unix_time(sched, 1112)
    ),
    CreateUpdatedEvent(fbb,
        motis::ris::CreateEvent(fbb,
          fbb.CreateString("0000002"),
          1,
          fbb.CreateString("381"),
          motis::ris::EventType_Departure,
          unix_time(sched, 1110)
        ),
        unix_time(sched, 1110)
    )
  };
  auto trip_id = motis::ris::CreateIdEvent(fbb,
        fbb.CreateString("0000001"),
        1,
        unix_time(sched, 1010));
  // clang-format on

  fbb.Finish(
      CreateMessage(fbb, MessageUnion_DelayMessage,
                    motis::ris::CreateDelayMessage(fbb, trip_id, DelayType_Is,
                                                   fbb.CreateVector(events))
                        .Union()));
}

msg_ptr get_ris_message(motis::schedule const& sched) {
  FlatBufferBuilder is_msg_fbb;
  create_is_msg(sched, is_msg_fbb);

  message_creator mc;
  std::vector<Offset<MessageHolder>> messages{CreateMessageHolder(
      mc,
      mc.CreateVector(is_msg_fbb.GetBufferPointer(), is_msg_fbb.GetSize()))};
  mc.create_and_finish(MsgContent_RISBatch,
                       CreateRISBatch(mc, mc.CreateVector(messages)).Union(),
                       "/ris/messages", DestinationType_Topic);
  return make_msg(mc);
}

}  // namespace invalid_realtime
}  // namespace schedule
}  // namespace test
}  // namespace motis
