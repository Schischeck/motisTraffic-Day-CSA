#include "motis/ris/risml_parser.h"

#include <iostream>
#include <map>

#include "boost/date_time.hpp"

#include "pugixml.hpp"

#include "parser/arg_parser.h"
#include "parser/cstr.h"

#include "motis/protocol/RISMessage_generated.h"

using namespace std::placeholders;
using namespace flatbuffers;
using namespace pugi;
using namespace parser;
using namespace motis::module;
using namespace motis::ris;

std::time_t parse_time(cstr const& raw) {
  // format YYYYMMDDhhmmssfff (fff = millis)
  std::tm time_struct;
  time_struct.tm_year = parse<int>(raw.substr(0, size(4))) - 1900;
  time_struct.tm_mon = parse<int>(raw.substr(4, size(2))) - 1;
  time_struct.tm_mday = parse<int>(raw.substr(6, size(2)));

  time_struct.tm_hour = parse<int>(raw.substr(8, size(2)));
  time_struct.tm_min = parse<int>(raw.substr(10, size(2)));
  time_struct.tm_sec = parse<int>(raw.substr(12, size(2)));
  time_struct.tm_wday = -1;
  time_struct.tm_yday = -1;
  time_struct.tm_isdst = -1;

  return std::mktime(&time_struct);
}

EventType parse_event_type(cstr const& raw) {
  static std::map<cstr, EventType> map({{"Start", EventType_Departure},
                                        {"Ab", EventType_Departure},
                                        {"Durch", EventType_Pass},
                                        {"An", EventType_Arrival},
                                        {"Ziel", EventType_Arrival}});
  return map[raw];
}

xml_attribute child_attr(xml_node const& n, char const* e, char const* a) {
  return n.child(e).attribute(a);
}

template <typename F>
void foreach_event(FlatBufferBuilder& fbb, xml_node const& msg, F func,
                   char const* train_selector = "./Service/ListZug/Zug") {
  for (auto&& train : msg.select_nodes(train_selector)) {
    auto const& t_node = train.node();
    auto train_index = t_node.attribute("Nr").as_int();

    for (auto&& train_event : train.node().select_nodes("./ListZE/ZE")) {
      auto const& e_node = train_event.node();

      StationIdType station_id_type;
      char const* station_id;
      auto const& eva_attribute = child_attr(e_node, "Bf", "EvaNr");
      if (!eva_attribute.empty()) {
        station_id_type = StationIdType_EVA;
        station_id = eva_attribute.value();
      } else {
        station_id_type = StationIdType_DS100;
        station_id = child_attr(e_node, "Bf", "Code").value();
      }

      auto event_type = parse_event_type(e_node.attribute("Typ").value());
      auto scheduled = parse_time(child_attr(e_node, "Zeit", "Soll").value());

      // Event, xml_node ZE, xml_node Zug
      func(CreateEvent(fbb, station_id_type, fbb.CreateString(station_id),
                       train_index, event_type, scheduled),
           e_node, t_node);
    }
  }
}

Offset<Message> parse_delay_msg(FlatBufferBuilder& fbb, xml_node const& msg,
                                DelayType delay_type) {
  std::vector<Offset<UpdatedEvent>> events;
  foreach_event(fbb, msg, [&](Offset<Event> const& event,
                              xml_node const& e_node, xml_node const&) {
    auto attr_name = (delay_type == DelayType_Is) ? "Ist" : "Prog";
    auto updated = parse_time(child_attr(e_node, "Zeit", attr_name).value());
    events.push_back(CreateUpdatedEvent(fbb, event, updated));
  });
  return CreateMessage(
      fbb, MessageUnion_DelayMessage,
      CreateDelayMessage(fbb, delay_type, fbb.CreateVector(events)).Union());
}

Offset<Message> parse_cancel_msg(FlatBufferBuilder& fbb, xml_node const& msg) {
  std::vector<Offset<Event>> events;
  foreach_event(fbb, msg, [&](Offset<Event> const& event, xml_node const&,
                              xml_node const&) { events.push_back(event); });
  return CreateMessage(
      fbb, MessageUnion_CancelMessage,
      CreateCancelMessage(fbb, fbb.CreateVector(events)).Union());
}

Offset<AdditionalEvent> parse_additional_event(FlatBufferBuilder& fbb,
                                               Offset<Event> const& event,
                                               xml_node const& e_node,
                                               xml_node const& t_node) {
  auto track_attr = child_attr(e_node, "Gleis", "Soll");
  auto track = (track_attr) ? fbb.CreateString(track_attr.value()) : 0;
  auto category = fbb.CreateString(t_node.attribute("Gattung").value());
  return CreateAdditionalEvent(fbb, event, category, track);
}

Offset<Message> parse_addition_msg(FlatBufferBuilder& fbb,
                                   xml_node const& msg) {
  std::vector<Offset<AdditionalEvent>> events;
  foreach_event(fbb, msg, [&](Offset<Event> const& event,
                              xml_node const& e_node, xml_node const& t_node) {
    events.push_back(parse_additional_event(fbb, event, e_node, t_node));
  });
  return CreateMessage(
      fbb, MessageUnion_AdditionMessage,
      CreateAdditionMessage(fbb, fbb.CreateVector(events)).Union());
}

Offset<Message> parse_reroute_msg(FlatBufferBuilder& fbb, xml_node const& msg) {
  std::vector<Offset<Event>> cancelled_events;
  foreach_event(fbb, msg,
                [&](Offset<Event> const& event, xml_node const&,
                    xml_node const&) { cancelled_events.push_back(event); });

  std::vector<Offset<ReroutedEvent>> new_events;
  foreach_event(
      fbb, msg,
      [&](Offset<Event> const& event, xml_node const& e_node,
          xml_node const& t_node) {
        auto additional = parse_additional_event(fbb, event, e_node, t_node);
        cstr status_str = e_node.attribute("RegSta").value();
        auto status = (status_str == "Normal") ? RerouteStatus_Normal
                                               : RerouteStatus_UmlNeu;
        new_events.push_back(CreateReroutedEvent(fbb, additional, status));
      },
      "./Service/ListUml/Uml/ListZug/Zug");

  return CreateMessage(
      fbb, MessageUnion_RerouteMessage,
      CreateRerouteMessage(fbb, fbb.CreateVector(cancelled_events),
                           fbb.CreateVector(new_events))
          .Union());
}

Offset<Packet> parse_xml(FlatBufferBuilder& fbb, char const* xml_string) {
  xml_document doc;
  xml_parse_result result = doc.load_string(xml_string);
  if (!result) {
    throw std::runtime_error("bad xml: " + std::string(result.description()));
  }

  std::vector<Offset<Message>> messages;
  for (auto const& msg : doc.select_nodes("/Paket/ListNachricht/Nachricht")) {
    using parser_t =
        std::function<Offset<Message>(FlatBufferBuilder&, xml_node const&)>;
    static std::map<cstr, parser_t> map(
        {{"Ist", std::bind(parse_delay_msg, _1, _2, DelayType_Is)},
         {"IstProg", std::bind(parse_delay_msg, _1, _2, DelayType_Forecast)},
         {"Ausfall", std::bind(parse_cancel_msg, _1, _2)},
         {"Zusatzzug", std::bind(parse_addition_msg, _1, _2)},
         {"Umleitung", std::bind(parse_reroute_msg, _1, _2)}});

    auto const& payload = msg.node().first_child();
    auto it = map.find(payload.name());
    if (it != map.end()) {
      messages.push_back(it->second(fbb, payload));
    }
    // TODO else: handle unknown message correctly
  }

  auto tout = parse_time(doc.child("Paket").attribute("TOut").value());
  return CreatePacket(fbb, tout, fbb.CreateVector(messages));
}

msg_ptr motis::ris::parse_xmls(std::vector<char const*> const& xml_strings) {
  FlatBufferBuilder fbb;

  std::vector<Offset<Packet>> packets;
  for (auto const& xml_string : xml_strings) {
    packets.push_back(parse_xml(fbb, xml_string));
  }

  fbb.Finish(motis::CreateMessage(
      fbb, MsgContent_RISBatch,
      CreateRISBatch(fbb, fbb.CreateVector(packets)).Union()));

  return make_msg(fbb);
}
