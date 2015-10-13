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

template <typename F>
void foreach_event(xml_node const& msg, FlatBufferBuilder& fbb, F func) {
  for (auto&& train : msg.select_nodes("./Service/ListZug/Zug")) {
    auto train_index = train.node().attribute("Nr").as_int();

    for (auto&& train_event : train.node().select_nodes("./ListZE/ZE")) {
      auto const& node = train_event.node();

      StationIdType station_id_type;
      char const* station_id;
      auto const& eva_node = node.child("Bf").attribute("EvaNr");
      if (!eva_node.empty()) {
        station_id_type = StationIdType_EVA;
        station_id = eva_node.value();
      } else {
        station_id_type = StationIdType_DS100;
        station_id = node.child("Bf").attribute("Code").value();
      }

      auto event_type = parse_event_type(node.attribute("Typ").value());
      auto scheduled = parse_time(node.child("Zeit").attribute("Soll").value());

      func(node, CreateEvent(fbb, station_id_type, fbb.CreateString(station_id),
                             train_index, event_type, scheduled));
    }
  }
}

Offset<Message> parse_delay_msg(FlatBufferBuilder& fbb, xml_node const& msg,
                                DelayType delay_type) {
  std::vector<Offset<UpdatedTrainEvent>> events;
  auto parse = [&](xml_node const& node, Offset<Event> const& event) {
    auto attr_name = (delay_type == DelayType_Is) ? "Ist" : "Prog";
    auto updated = parse_time(node.child("Zeit").attribute(attr_name).value());
    events.push_back(CreateUpdatedTrainEvent(fbb, event, updated));
  };

  foreach_event(msg, fbb, parse);
  return CreateMessage(
      fbb, MessageUnion_DelayMessage,
      CreateDelayMessage(fbb, delay_type, fbb.CreateVector(events)).Union());
}

Offset<Message> parse_cancel_msg(FlatBufferBuilder& fbb, xml_node const& msg) {
  std::vector<Offset<Event>> events;
  foreach_event(msg, fbb, [&](xml_node const&, Offset<Event> const& event) {
    events.push_back(event);
  });
  return CreateMessage(
      fbb, MessageUnion_CancelMessage,
      CreateCancelMessage(fbb, fbb.CreateVector(events)).Union());
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
         {"Ausfall", std::bind(parse_cancel_msg, _1, _2)}});
    // TODO handle unknown message!?

    auto const& payload = msg.node().first_child();
    messages.push_back(map[payload.name()](fbb, payload));
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
