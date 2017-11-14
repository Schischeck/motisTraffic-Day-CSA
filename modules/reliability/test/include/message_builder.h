#pragma once

#include "motis/module/message.h"

#include "motis/protocol/RISMessage_generated.h"

namespace motis {
namespace reliability {
namespace realtime {

inline module::msg_ptr get_delay_message(
    std::string const& station, unsigned const train_nr,
    std::string const line_id, EventType event_type,
    time_t const scheduled_time, time_t const delayed_time,
    std::string const& trip_station, unsigned const trip_train_nr,
    time_t const trip_scheduled_time, ris::DelayType const delay_type) {
  using namespace ::flatbuffers;
  using namespace ris;
  FlatBufferBuilder fbb;
  std::vector<Offset<UpdatedEvent>> events{CreateUpdatedEvent(
      fbb,
      CreateEvent(fbb, fbb.CreateString(station), train_nr,
                  fbb.CreateString(line_id), event_type, scheduled_time),
      delayed_time)};
  fbb.Finish(CreateMessage(
      fbb, MessageUnion_DelayMessage,
      CreateDelayMessage(
          fbb,
          ris::CreateIdEvent(fbb, fbb.CreateString(trip_station), trip_train_nr,
                             trip_scheduled_time, IdEventType_Schedule),
          delay_type, fbb.CreateVector(events))
          .Union()));

  module::message_creator mc;
  std::vector<Offset<MessageHolder>> messages{CreateMessageHolder(
      mc, mc.CreateVector(fbb.GetBufferPointer(), fbb.GetSize()))};
  mc.create_and_finish(MsgContent_RISBatch,
                       CreateRISBatch(mc, mc.CreateVector(messages)).Union(),
                       "/ris/messages");
  return make_msg(mc);
}

}  // namespace realtime
}  // namespace reliability
}  // namespace motis
