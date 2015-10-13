#include "motis/ris/risml_parser.h"

#include <iostream>
#include <map>

#include "boost/date_time.hpp"

#include "pugixml.hpp"

#include "parser/arg_parser.h"
#include "parser/cstr.h"

#include "motis/protocol/RISMessage_generated.h"

namespace fb = flatbuffers;
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

fb::Offset<Packet> parse_xml(char const* xml_string,
                             fb::FlatBufferBuilder& fbb) {
  pugi::xml_document doc;
  pugi::xml_parse_result result = doc.load_string(xml_string);
  if (!result) {
    throw std::runtime_error("bad xml: " + std::string(result.description()));
  }

  std::vector<fb::Offset<Message>> messages;
  for (auto const& msg : doc.select_nodes("/Paket/ListNachricht/Nachricht")) {

    std::vector<fb::Offset<UpdatedTrainEvent>> events;
    for (auto const& train :
         msg.node().select_nodes("./Ist/Service/ListZug/Zug")) {
      auto train_index = train.node().attribute("Nr").as_int();

      for (auto const& train_event : train.node().select_nodes("./ListZE/ZE")) {
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

        auto const& time = node.child("Zeit");
        auto scheduled = parse_time(time.attribute("Soll").value());
        auto updated = parse_time(time.attribute("Ist").value());

        auto base =
            CreateEvent(fbb, station_id_type, fbb.CreateString(station_id),
                        train_index, event_type, scheduled);
        events.push_back(CreateUpdatedTrainEvent(fbb, base, updated));
      }
    }

    auto delayMessage = CreateDelayMessage(
        fbb, DelayType_Is,
        fbb.CreateVector<fb::Offset<UpdatedTrainEvent>>({events}));
    messages.push_back(
        CreateMessage(fbb, MessageUnion_DelayMessage, delayMessage.Union()));
  }

  auto tout = parse_time(doc.child("Paket").attribute("TOut").value());
  return CreatePacket(fbb, tout, fbb.CreateVector(messages));
}

msg_ptr motis::ris::parse_xmls(std::vector<char const*> const& xml_strings) {
  fb::FlatBufferBuilder fbb;

  std::vector<fb::Offset<Packet>> packets;
  for (auto const& xml_string : xml_strings) {
    packets.push_back(parse_xml(xml_string, fbb));
  }

  fbb.Finish(motis::CreateMessage(
      fbb, MsgContent_RISBatch,
      CreateRISBatch(fbb, fbb.CreateVector(packets)).Union()));

  return make_msg(fbb);
}