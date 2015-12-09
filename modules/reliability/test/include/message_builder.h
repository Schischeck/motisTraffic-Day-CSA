#pragma once

#include "motis/module/message.h"

namespace motis {
namespace reliability {
namespace realtime {
inline module::msg_ptr get_ris_message(std::string const& station,
                                       unsigned const train_nr,
                                       time_t const scheduled_time,
                                       time_t const delayed_time,
                                       ris::EventType event_type,
                                       ris::DelayType const delayType) {
  using namespace flatbuffers;
  using namespace ris;
  FlatBufferBuilder fbb;
  // clang-format off
    std::vector<Offset<UpdatedEvent>> events{
      CreateUpdatedEvent(fbb,
          CreateEvent(fbb,
            StationIdType_EVA,
            fbb.CreateString(station),
            train_nr,
            event_type,
            scheduled_time
          ),
          delayed_time
      )};
  // clang-format on
  fbb.Finish(CreateMessage(
      fbb, MessageUnion_DelayMessage,
      CreateDelayMessage(fbb, delayType, fbb.CreateVector(events)).Union()));

  module::MessageCreator mc;
  std::vector<Offset<MessageHolder>> messages{CreateMessageHolder(
      mc, mc.CreateVector(fbb.GetBufferPointer(), fbb.GetSize()))};
  mc.CreateAndFinish(MsgContent_RISBatch,
                     CreateRISBatch(mc, mc.CreateVector(messages)).Union());
  return make_msg(mc);
}
}
}
}
