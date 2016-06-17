#pragma once

#include "motis/module/message.h"

namespace motis {
namespace reliability {
namespace realtime {

// TODO line_ids and trip_ids are stubs right now
inline ::flatbuffers::Offset<ris::TripId> stub_id(
    ::flatbuffers::FlatBufferBuilder& fbb) {
  using namespace ris;
  return CreateTripId(fbb,
                      CreateEvent(fbb, StationIdType_EVA, fbb.CreateString(""),
                                  0, fbb.CreateString(""), EventType_DEP, 0),
                      StationIdType_EVA, fbb.CreateString(""), 0);
}

inline module::msg_ptr get_delay_message(std::string const& station,
                                         unsigned const train_nr,
                                         time_t const scheduled_time,
                                         time_t const delayed_time,
                                         ris::EventType event_type,
                                         ris::DelayType const delayType) {
  using namespace ::flatbuffers;
  using namespace ris;
  FlatBufferBuilder fbb;
  // clang-format off
    std::vector<Offset<UpdatedEvent>> events{
      CreateUpdatedEvent(fbb,
          CreateEvent(fbb,
            StationIdType_EVA,
            fbb.CreateString(station),
            train_nr,
            fbb.CreateString("TODO"),
            event_type,
            scheduled_time
          ),
          delayed_time
      )};
  // clang-format on
  fbb.Finish(CreateMessage(
      fbb, MessageUnion_DelayMessage,
      CreateDelayMessage(fbb, stub_id(fbb), delayType, fbb.CreateVector(events))
          .Union()));

  module::message_creator mc;
  std::vector<Offset<MessageHolder>> messages{CreateMessageHolder(
      mc, mc.CreateVector(fbb.GetBufferPointer(), fbb.GetSize()))};
  mc.create_and_finish(MsgContent_RISBatch,
                       CreateRISBatch(mc, mc.CreateVector(messages)).Union());
  return make_msg(mc);
}

struct event {
  std::string station;
  unsigned train_nr;
  time_t scheduled_time;
  ris::EventType event_type;
};
inline module::msg_ptr get_cancel_message(std::vector<event> const& events) {
  using namespace ::flatbuffers;
  using namespace ris;
  FlatBufferBuilder fbb;
  std::vector<Offset<Event>> o_events;
  for (auto const& e : events) {
    o_events.push_back(CreateEvent(
        fbb, StationIdType_EVA, fbb.CreateString(e.station), e.train_nr,
        fbb.CreateString("TODO"), e.event_type, e.scheduled_time));
  }
  fbb.Finish(CreateMessage(
      fbb, MessageUnion_CancelMessage,
      CreateCancelMessage(fbb, stub_id(fbb), fbb.CreateVector(o_events))
          .Union()));

  module::message_creator mc;
  std::vector<Offset<MessageHolder>> messages{CreateMessageHolder(
      mc, mc.CreateVector(fbb.GetBufferPointer(), fbb.GetSize()))};
  mc.create_and_finish(MsgContent_RISBatch,
                       CreateRISBatch(mc, mc.CreateVector(messages)).Union());
  return make_msg(mc);
}

struct rerouted_event : event {
  rerouted_event(event const& e, std::string cat, std::string track)
      : category(cat), track(track) {
    station = e.station;
    train_nr = e.train_nr;
    scheduled_time = e.scheduled_time;
    event_type = e.event_type;
  }
  std::string category;
  std::string track;
};

inline module::msg_ptr get_reroute_message(
    std::vector<event> const& cancelled_events,
    std::vector<rerouted_event> const& new_events) {
  using namespace ::flatbuffers;
  using namespace ris;
  FlatBufferBuilder fbb;
  std::vector<Offset<Event>> o_cancelled_events;
  for (auto const& e : cancelled_events) {
    o_cancelled_events.push_back(CreateEvent(
        fbb, StationIdType_EVA, fbb.CreateString(e.station), e.train_nr,
        fbb.CreateString("TODO"), e.event_type, e.scheduled_time));
  }

  std::vector<Offset<ReroutedEvent>> o_rerouted_events;
  for (auto const& e : new_events) {
    o_rerouted_events.push_back(CreateReroutedEvent(
        fbb, CreateAdditionalEvent(
                 fbb, CreateEvent(fbb, StationIdType_EVA,
                                  fbb.CreateString(e.station), e.train_nr,
                                  fbb.CreateString("TODO"), e.event_type,
                                  e.scheduled_time),
                 fbb.CreateString(e.category), fbb.CreateString(e.track)),
        RerouteStatus_UmlNeu));
  }

  fbb.Finish(
      CreateMessage(fbb, MessageUnion_RerouteMessage,
                    CreateRerouteMessage(fbb, stub_id(fbb),
                                         fbb.CreateVector(o_cancelled_events),
                                         fbb.CreateVector(o_rerouted_events))
                        .Union()));

  module::message_creator mc;
  std::vector<Offset<MessageHolder>> messages{CreateMessageHolder(
      mc, mc.CreateVector(fbb.GetBufferPointer(), fbb.GetSize()))};
  mc.create_and_finish(MsgContent_RISBatch,
                       CreateRISBatch(mc, mc.CreateVector(messages)).Union());
  return make_msg(mc);
}
}
}
}
