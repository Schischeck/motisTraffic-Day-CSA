#include "motis/test/schedule/simple_realtime.h"

#include "motis/core/schedule/schedule.h"
#include "motis/core/access/time_access.h"

using namespace flatbuffers;
using namespace motis::module;
using namespace motis::ris;

namespace motis {
namespace test {
namespace schedule {
namespace simple_realtime {

void create_is_msg(motis::schedule const& sched, FlatBufferBuilder& fbb) {
  // clang-format off
  std::vector<Offset<UpdatedEvent>> events{
    CreateUpdatedEvent(fbb,
        motis::ris::CreateEvent(fbb,
          fbb.CreateString("8000010"),  // Aschaffenburg
          628,
          fbb.CreateString(""),
          motis::ris::EventType_Departure,
          unix_time(sched, 1436)
        ),
        unix_time(sched, 1437)
    ),
    CreateUpdatedEvent(fbb,
        motis::ris::CreateEvent(fbb,
          fbb.CreateString("8070003"),  // Frankfurt(M) Flughafen
          628,
          fbb.CreateString(""),
          motis::ris::EventType_Departure,
          unix_time(sched, 1525)
        ),
        unix_time(sched, 1530)
    )};
  auto trip_id = motis::ris::CreateIdEvent(fbb,
        fbb.CreateString("8000261"),  // Muenchen Hbf
        628,
        unix_time(sched, 1154));
  // clang-format on

  fbb.Finish(
      CreateMessage(fbb, MessageUnion_DelayMessage,
                    motis::ris::CreateDelayMessage(fbb, trip_id, DelayType_Is,
                                                   fbb.CreateVector(events))
                        .Union()));
}

void create_forecast_msg(motis::schedule const& sched, FlatBufferBuilder& fbb) {
  // clang-format off
  std::vector<Offset<UpdatedEvent>> events{
    CreateUpdatedEvent(fbb,
        motis::ris::CreateEvent(fbb,
          fbb.CreateString("8000105"),  // Frankfurt(Main)Hbf
          628,
          fbb.CreateString(""),
          motis::ris::EventType_Arrival,
          unix_time(sched, 1504)
        ),
        unix_time(sched, 1505)
    ),
    CreateUpdatedEvent(fbb,
        motis::ris::CreateEvent(fbb,
          fbb.CreateString("8073368"),  // Köln Messe/Deutz Gl.1
          628,
          fbb.CreateString(""),
          motis::ris::EventType_Arrival,
          unix_time(sched, 1614)
        ),
        unix_time(sched, 1619)
    )};
  auto trip_id = motis::ris::CreateIdEvent(fbb,
        fbb.CreateString("8000261"),  // Muenchen Hbf
        628,
        unix_time(sched, 1154));
  // clang-format on

  fbb.Finish(CreateMessage(
      fbb, MessageUnion_DelayMessage,
      motis::ris::CreateDelayMessage(fbb, trip_id, DelayType_Forecast,
                                     fbb.CreateVector(events))
          .Union()));
}

msg_ptr get_ris_message(motis::schedule const& sched) {
  FlatBufferBuilder is_msg_fbb;
  create_is_msg(sched, is_msg_fbb);

  FlatBufferBuilder fc_msg_fbb;
  create_forecast_msg(sched, fc_msg_fbb);

  message_creator mc;
  std::vector<Offset<MessageHolder>> messages{
      CreateMessageHolder(mc, mc.CreateVector(is_msg_fbb.GetBufferPointer(),
                                              is_msg_fbb.GetSize())),
      CreateMessageHolder(mc, mc.CreateVector(fc_msg_fbb.GetBufferPointer(),
                                              fc_msg_fbb.GetSize()))};
  mc.create_and_finish(MsgContent_RISBatch,
                       CreateRISBatch(mc, mc.CreateVector(messages)).Union(),
                       "/ris/messages", DestinationType_Topic);
  return make_msg(mc);
}

}  // namespace simple_realtime
}  // namespace schedule
}  // namespace test
}  // namespace motis
