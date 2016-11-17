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

void create_invalid_trip_is_msg(motis::schedule const& sched,
                                FlatBufferBuilder& fbb) {
  // clang-format off
  std::vector<Offset<UpdatedEvent>> events{
    CreateUpdatedEvent(fbb,
        CreateEvent(fbb,
          fbb.CreateString("0000002"),
          1,
          fbb.CreateString("381"),
          EventType_DEP,
          unix_time(sched, 1110)
        ),
        unix_time(sched, 1301)
    ),
    CreateUpdatedEvent(fbb,
        CreateEvent(fbb,
          fbb.CreateString("0000004"),
          1,
          fbb.CreateString("381"),
          EventType_ARR,
          unix_time(sched, 1300)
        ),
        unix_time(sched, 1200)
    )
  };
  auto trip_id = CreateIdEvent(fbb,
        fbb.CreateString("0000001"),
        1,
        unix_time(sched, 1010));
  // clang-format on

  fbb.Finish(CreateMessage(
      fbb, MessageUnion_DelayMessage,
      CreateDelayMessage(fbb, trip_id, DelayType_Is, fbb.CreateVector(events))
          .Union()));
}

void create_invalid_ts_is_msg(motis::schedule const& sched,
                              FlatBufferBuilder& fbb) {
  // clang-format off
  std::vector<Offset<UpdatedEvent>> events{
    CreateUpdatedEvent(fbb,
        CreateEvent(fbb,
          fbb.CreateString("0000004"),
          1,
          fbb.CreateString("381"),
          EventType_DEP,
          unix_time(sched, 1310)
        ),
        unix_time(sched, 1200)
    )
  };
  auto trip_id = CreateIdEvent(fbb,
        fbb.CreateString("0000001"),
        1,
        unix_time(sched, 1010));
  // clang-format on

  fbb.Finish(CreateMessage(
      fbb, MessageUnion_DelayMessage,
      CreateDelayMessage(fbb, trip_id, DelayType_Is, fbb.CreateVector(events))
          .Union()));
}

void create_additional_msg(motis::schedule const& sched,
                           FlatBufferBuilder& fbb) {
  // clang-format off
  std::vector<Offset<AdditionalEvent>> events = {
    CreateAdditionalEvent(fbb,
      CreateEvent(fbb,
        fbb.CreateString("0000001"),
        77,
        fbb.CreateString(""),
        EventType_DEP,
        unix_time(sched, 2200)),
      fbb.CreateString("ICE"),
      fbb.CreateString("7a")
    ),
    CreateAdditionalEvent(fbb,
      CreateEvent(fbb,
        fbb.CreateString("0000003"),
        77,
        fbb.CreateString(""),
        EventType_ARR,
        unix_time(sched, 2225)),
      fbb.CreateString("ICE"),
      fbb.CreateString("2")
    ),
    CreateAdditionalEvent(fbb,
      CreateEvent(fbb,
        fbb.CreateString("0000003"),
        77,
        fbb.CreateString(""),
        EventType_DEP,
        unix_time(sched, 2230)),
      fbb.CreateString("ICE"),
      fbb.CreateString("2")
    ),
    CreateAdditionalEvent(fbb,
      CreateEvent(fbb,
        fbb.CreateString("0000004"),
        77,
        fbb.CreateString(""),
        EventType_ARR,
        unix_time(sched, 2300)),
      fbb.CreateString("ICE"),
      fbb.CreateString("5")
    )
  };
  // clang-format on
  auto trip_id = CreateIdEvent(fbb, fbb.CreateString("0000001"), 77,
                               unix_time(sched, 2200));
  fbb.Finish(CreateMessage(
      fbb, MessageUnion_AdditionMessage,
      CreateAdditionMessage(fbb, trip_id, fbb.CreateVector(events)).Union()));
}

void create_cancel_msg(motis::schedule const& sched, FlatBufferBuilder& fbb) {
  // clang-format off
  std::vector<Offset<Event>> events{
    CreateEvent(fbb,
      fbb.CreateString("0000003"),
      1,
      fbb.CreateString("381"),
      EventType_DEP,
      unix_time(sched, 1210)),
    CreateEvent(fbb,
      fbb.CreateString("0000004"),
      1,
      fbb.CreateString("381"),
      EventType_ARR,
      unix_time(sched, 1300)
    ),
    CreateEvent(fbb,
      fbb.CreateString("0000004"),
      1,
      fbb.CreateString("381"),
      EventType_DEP,
      unix_time(sched, 1310)
    ),
    CreateEvent(fbb,
      fbb.CreateString("0000005"),
      1,
      fbb.CreateString("381"),
      EventType_ARR,
      unix_time(sched, 1400)
    )
  };
  auto trip_id = CreateIdEvent(fbb,
        fbb.CreateString("0000001"),
        1,
        unix_time(sched, 1010));
  // clang-format on

  fbb.Finish(CreateMessage(
      fbb, MessageUnion_CancelMessage,
      CreateCancelMessage(fbb, trip_id, fbb.CreateVector(events)).Union()));
}

void create_delay_for_reroute_msg(motis::schedule const& sched,
                                  FlatBufferBuilder& fbb) {
  // clang-format off
  std::vector<Offset<UpdatedEvent>> events{
    CreateUpdatedEvent(fbb,
        CreateEvent(fbb,
          fbb.CreateString("0000002"),
          1,
          fbb.CreateString("381"),
          EventType_ARR,
          unix_time(sched, 1100)
        ),
        unix_time(sched, 1105)
    ),
    CreateUpdatedEvent(fbb,
        CreateEvent(fbb,
          fbb.CreateString("0000002"),
          1,
          fbb.CreateString("381"),
          EventType_DEP,
          unix_time(sched, 1110)
        ),
        unix_time(sched, 1112)
    ),
    CreateUpdatedEvent(fbb,
        CreateEvent(fbb,
          fbb.CreateString("0000003"),
          1,
          fbb.CreateString("381"),
          EventType_ARR,
          unix_time(sched, 1200)
        ),
        unix_time(sched, 1202)
    ),
    CreateUpdatedEvent(fbb,
        CreateEvent(fbb,
          fbb.CreateString("0000003"),
          1,
          fbb.CreateString("381"),
          EventType_DEP,
          unix_time(sched, 1210)
        ),
        unix_time(sched, 1212)
    ),
    CreateUpdatedEvent(fbb,
        CreateEvent(fbb,
          fbb.CreateString("0000004"),
          1,
          fbb.CreateString("381"),
          EventType_ARR,
          unix_time(sched, 1300)
        ),
        unix_time(sched, 1305)
    ),
    CreateUpdatedEvent(fbb,
        CreateEvent(fbb,
          fbb.CreateString("0000004"),
          1,
          fbb.CreateString("381"),
          EventType_DEP,
          unix_time(sched, 1310)
        ),
        unix_time(sched, 1312)
    ),
    CreateUpdatedEvent(fbb,
        CreateEvent(fbb,
          fbb.CreateString("0000005"),
          1,
          fbb.CreateString("381"),
          EventType_ARR,
          unix_time(sched, 1400)
        ),
        unix_time(sched, 1405)
    )
  };
  auto trip_id = CreateIdEvent(fbb,
        fbb.CreateString("0000001"),
        1,
        unix_time(sched, 1010));
  // clang-format on

  fbb.Finish(CreateMessage(fbb, MessageUnion_DelayMessage,
                           CreateDelayMessage(fbb, trip_id, DelayType_Forecast,
                                              fbb.CreateVector(events))
                               .Union()));
}

void create_cancel_reroute_msg(motis::schedule const& sched,
                               FlatBufferBuilder& fbb) {
  // clang-format off
  std::vector<Offset<Event>> cancelled_events{
    CreateEvent(fbb,
      fbb.CreateString("0000002"),
      1,
      fbb.CreateString("381"),
      EventType_ARR,
      unix_time(sched, 1100)),
    CreateEvent(fbb,
      fbb.CreateString("0000002"),
      1,
      fbb.CreateString("381"),
      EventType_DEP,
      unix_time(sched, 1110)
    )
  };

  std::vector<Offset<ReroutedEvent>> new_events{};

  auto trip_id = CreateIdEvent(fbb,
        fbb.CreateString("0000001"),
        1,
        unix_time(sched, 1010));
  // clang-format on

  fbb.Finish(CreateMessage(
      fbb, MessageUnion_RerouteMessage,
      CreateRerouteMessage(fbb, trip_id, fbb.CreateVector(cancelled_events),
                           fbb.CreateVector(new_events))
          .Union()));
}

void create_reroute_msg(motis::schedule const& sched, FlatBufferBuilder& fbb) {
  // clang-format off
  std::vector<Offset<Event>> cancelled_events{
    CreateEvent(fbb,
      fbb.CreateString("0000001"),
      1,
      fbb.CreateString("381"),
      EventType_DEP,
      unix_time(sched, 1010)),
    CreateEvent(fbb,
      fbb.CreateString("0000003"),
      1,
      fbb.CreateString("381"),
      EventType_ARR,
      unix_time(sched, 1200)
    ),
    CreateEvent(fbb,
      fbb.CreateString("0000003"),
      1,
      fbb.CreateString("381"),
      EventType_DEP,
      unix_time(sched, 1210)
    ),
    CreateEvent(fbb,
      fbb.CreateString("0000005"),
      1,
      fbb.CreateString("381"),
      EventType_ARR,
      unix_time(sched, 1400)
    )
  };

  std::vector<Offset<ReroutedEvent>> new_events{
    CreateReroutedEvent(fbb,
      CreateAdditionalEvent(fbb,
        CreateEvent(fbb,
          fbb.CreateString("0000002"),
          1,
          fbb.CreateString("381"),
          EventType_ARR,
          unix_time(sched, 1100)
        ),
        fbb.CreateString("IC"),
        fbb.CreateString("9")
      ),
      RerouteStatus_Normal
    ),
    CreateReroutedEvent(fbb,
      CreateAdditionalEvent(fbb,
        CreateEvent(fbb,
          fbb.CreateString("0000002"),
          1,
          fbb.CreateString("381"),
          EventType_DEP,
          unix_time(sched, 1110)
        ),
        fbb.CreateString("IC"),
        fbb.CreateString("9")
      ),
      RerouteStatus_Normal
    ),
    CreateReroutedEvent(fbb,
      CreateAdditionalEvent(fbb,
        CreateEvent(fbb,
          fbb.CreateString("0000005"),
          1,
          fbb.CreateString("381"),
          EventType_DEP,
          unix_time(sched, 910)
        ),
        fbb.CreateString("IC"),
        fbb.CreateString("9")
      ),
      RerouteStatus_UmlNeu
    ),
    CreateReroutedEvent(fbb,
      CreateAdditionalEvent(fbb,
        CreateEvent(fbb,
          fbb.CreateString("0000001"),
          1,
          fbb.CreateString("381"),
          EventType_ARR,
          unix_time(sched, 1500)
        ),
        fbb.CreateString("IC"),
        fbb.CreateString("9")
      ),
      RerouteStatus_UmlNeu
    )
  };

  auto trip_id = CreateIdEvent(fbb,
        fbb.CreateString("0000001"),
        1,
        unix_time(sched, 1010));
  // clang-format on

  fbb.Finish(CreateMessage(
      fbb, MessageUnion_RerouteMessage,
      CreateRerouteMessage(fbb, trip_id, fbb.CreateVector(cancelled_events),
                           fbb.CreateVector(new_events))
          .Union()));
}

msg_ptr get_trip_conflict_ris_message(motis::schedule const& sched) {
  FlatBufferBuilder is_msg_fbb;
  create_invalid_trip_is_msg(sched, is_msg_fbb);

  message_creator mc;
  std::vector<Offset<MessageHolder>> messages{CreateMessageHolder(
      mc,
      mc.CreateVector(is_msg_fbb.GetBufferPointer(), is_msg_fbb.GetSize()))};
  mc.create_and_finish(MsgContent_RISBatch,
                       CreateRISBatch(mc, mc.CreateVector(messages)).Union(),
                       "/ris/messages", DestinationType_Topic);
  return make_msg(mc);
}

msg_ptr get_ts_conflict_ris_message(motis::schedule const& sched) {
  FlatBufferBuilder is_msg_fbb;
  create_invalid_ts_is_msg(sched, is_msg_fbb);

  message_creator mc;
  std::vector<Offset<MessageHolder>> messages{CreateMessageHolder(
      mc,
      mc.CreateVector(is_msg_fbb.GetBufferPointer(), is_msg_fbb.GetSize()))};
  mc.create_and_finish(MsgContent_RISBatch,
                       CreateRISBatch(mc, mc.CreateVector(messages)).Union(),
                       "/ris/messages", DestinationType_Topic);
  return make_msg(mc);
}

motis::module::msg_ptr get_additional_ris_message(
    motis::schedule const& sched) {
  FlatBufferBuilder fbb;
  create_additional_msg(sched, fbb);

  message_creator mc;
  std::vector<Offset<MessageHolder>> messages{CreateMessageHolder(
      mc, mc.CreateVector(fbb.GetBufferPointer(), fbb.GetSize()))};
  mc.create_and_finish(MsgContent_RISBatch,
                       CreateRISBatch(mc, mc.CreateVector(messages)).Union(),
                       "/ris/messages", DestinationType_Topic);
  return make_msg(mc);
}

motis::module::msg_ptr get_cancel_ris_message(motis::schedule const& sched) {
  FlatBufferBuilder fbb;
  create_cancel_msg(sched, fbb);

  message_creator mc;
  std::vector<Offset<MessageHolder>> messages{CreateMessageHolder(
      mc, mc.CreateVector(fbb.GetBufferPointer(), fbb.GetSize()))};
  mc.create_and_finish(MsgContent_RISBatch,
                       CreateRISBatch(mc, mc.CreateVector(messages)).Union(),
                       "/ris/messages", DestinationType_Topic);
  return make_msg(mc);
}

motis::module::msg_ptr get_reroute_ris_message(motis::schedule const& sched) {
  FlatBufferBuilder fbb0;
  create_delay_for_reroute_msg(sched, fbb0);

  FlatBufferBuilder fbb1;
  create_cancel_reroute_msg(sched, fbb1);

  FlatBufferBuilder fbb2;
  create_reroute_msg(sched, fbb2);

  message_creator mc;
  std::vector<Offset<MessageHolder>> messages{
      CreateMessageHolder(
          mc, mc.CreateVector(fbb0.GetBufferPointer(), fbb0.GetSize())),
      CreateMessageHolder(
          mc, mc.CreateVector(fbb1.GetBufferPointer(), fbb1.GetSize())),
      CreateMessageHolder(
          mc, mc.CreateVector(fbb2.GetBufferPointer(), fbb2.GetSize()))};
  mc.create_and_finish(MsgContent_RISBatch,
                       CreateRISBatch(mc, mc.CreateVector(messages)).Union(),
                       "/ris/messages", DestinationType_Topic);
  return make_msg(mc);
}

}  // namespace invalid_realtime
}  // namespace schedule
}  // namespace test
}  // namespace motis
