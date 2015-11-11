#include "gtest/gtest.h"

#include "include/helper.h"

#include "motis/protocol/RISMessage_generated.h"
#include "motis/ris/risml_parser.h"

namespace motis {
namespace ris {

// clang-format off
char const* ist_fixture_1 = "<?xml version=\"1.0\" encoding=\"iso-8859-1\" ?>\
<Paket Version=\"1.2\" SpezVer=\"1\" TOut=\"20151006235934781\" KNr=\"123863714\">\
<ListNachricht><Nachricht><Ist>\
<Service Id=\"85713913\" IdZNr=\"8329\" IdZGattung=\"S\" IdBf=\"MMAM\" \
IdBfEvaNr=\"8004204\" IdZeit=\"20151006234400\"  ZielBfCode=\"MHO\" \
ZielBfEvaNr=\"8002980\" Zielzeit=\"20151007010600\" IdVerwaltung=\"07\" \
IdZGattungInt=\"s\" IdLinie=\"3\" SourceZNr=\"EFZ\">\
<ListZug>\
<Zug Nr=\"8329\" Gattung=\"S\" Linie=\"3\"  GattungInt=\"s\" Verwaltung=\"07\" >\
<ListZE>\
<ZE Typ=\"Ab\" >\
<Bf Code=\"MOL\" EvaNr=\"8004667\" Name=\"Olching\" />\
<Zeit Soll=\"20151006235900\" Ist=\"20151006235900\" />\
</ZE>    </ListZE></Zug></ListZug></Service></Ist><ListQuelle>\
<Quelle Sender=\"ZENTRAL\"  Typ=\"IstProg\" KNr=\"18762\" TIn=\"20151006235920176\" \
TOutSnd=\"20151006235934696\"/>\
<Quelle Sender=\"10.35.205.140:7213/13\" Typ=\"UIC 102\" TIn=\"20151006235933\" \
Esc=\"mue810jyct\" />\
</ListQuelle></Nachricht></ListNachricht></Paket>";
// clang-format on

TEST(delay_message, ist_message_1) {
  auto const messages = parse_xmls(pack(ist_fixture_1));
  ASSERT_EQ(1, messages.size());

  auto const& message = messages[0];
  EXPECT_EQ(1444168774, message.timestamp);
  EXPECT_EQ(1444172760, message.scheduled);

  auto outer_msg = GetMessage(message.data());
  ASSERT_EQ(MessageUnion_DelayMessage, outer_msg->content_type());
  auto inner_msg = reinterpret_cast<DelayMessage const*>(outer_msg->content());

  EXPECT_EQ(DelayType_Is, inner_msg->type());

  auto events = inner_msg->events();
  ASSERT_EQ(1, events->size());

  EXPECT_EQ(8329, events->Get(0)->base()->trainIndex());
  EXPECT_EQ(StationIdType_EVA, events->Get(0)->base()->stationIdType());
  EXPECT_EQ(std::string("8004667"),
            events->Get(0)->base()->stationId()->c_str());
  EXPECT_EQ(1444168740, events->Get(0)->base()->scheduledTime());
  EXPECT_EQ(EventType_Departure, events->Get(0)->base()->type());

  EXPECT_EQ(1444168740, events->Get(0)->updatedTime());
}

// clang-format off
char const* ist_fixture_2 = "<?xml version=\"1.0\" encoding=\"iso-8859-1\" ?>\
<Paket Version=\"1.2\" SpezVer=\"1\" TOut=\"20151007000002316\" KNr=\"123863963\">\
<ListNachricht><Nachricht><Ist>\
<Service Id=\"86009330\" IdZNr=\"60418\" IdZGattung=\"IC\" IdBf=\"MH\"\
 IdBfEvaNr=\"8000261\" IdZeit=\"20151006225000\"  ZielBfCode=\"TS\"\
 ZielBfEvaNr=\"8000096\" Zielzeit=\"20151007011600\" IdVerwaltung=\"80\"\
 IdZGattungInt=\"IC\" IdLinie=\"\" SourceZNr=\"EFZ\">\
<ListZug><Zug Nr=\"60418\" Gattung=\"IC\"  GattungInt=\"IC\" Verwaltung=\"80\" >\
<ListZE><ZE Typ=\"Ab\" ><Bf Code=\"MNFG\" />\
<Zeit Soll=\"20151006234800\" Ist=\"20151006235900\" />\
</ZE>    </ListZE></Zug></ListZug></Service></Ist><ListQuelle>\
<Quelle Sender=\"ZENTRAL\"  Typ=\"IstProg\" KNr=\"18777\" TIn=\"20151007000000336\"\
 TOutSnd=\"20151007000002053\"/><Quelle Sender=\"10.35.205.140:7213/13\"\
 Typ=\"UIC 102\" TIn=\"20151007000001\" Esc=\"mue810jyhi\" />\
</ListQuelle></Nachricht></ListNachricht></Paket>";
// clang-format on

TEST(delay_message, ist_message_2) {
  auto const messages = parse_xmls(pack(ist_fixture_2));
  ASSERT_EQ(1, messages.size());

  auto const& message = messages[0];
  EXPECT_EQ(1444168802, message.timestamp);
  EXPECT_EQ(1444173360, message.scheduled);

  auto outer_msg = GetMessage(message.data());
  ASSERT_EQ(MessageUnion_DelayMessage, outer_msg->content_type());
  auto inner_msg = reinterpret_cast<DelayMessage const*>(outer_msg->content());

  EXPECT_EQ(DelayType_Is, inner_msg->type());

  auto events = inner_msg->events();
  ASSERT_EQ(1, events->size());

  EXPECT_EQ(60418, events->Get(0)->base()->trainIndex());
  EXPECT_EQ(StationIdType_DS100, events->Get(0)->base()->stationIdType());
  EXPECT_EQ(std::string("MNFG"), events->Get(0)->base()->stationId()->c_str());
  EXPECT_EQ(1444168080, events->Get(0)->base()->scheduledTime());
  EXPECT_EQ(EventType_Departure, events->Get(0)->base()->type());

  EXPECT_EQ(1444168740, events->Get(0)->updatedTime());
}

// clang-format off
std::string type_fixture(std::string type_string) {
  return std::string("<?xml version=\"1.0\" encoding=\"iso-8859-1\" ?>\
<Paket TOut=\"12345678901234\"><ListNachricht><Nachricht>\
<Ist><Service Zielzeit=\"12345678901234\">\
<ListZug><Zug><ListZE><ZE Typ=\"") + type_string + "\" >\
<Bf/><Zeit Soll=\"12345678901234\" Ist=\"12345678901234\"/></ZE></ListZE>\
</Zug></ListZug></Service></Ist><ListQuelle>\
</ListQuelle></Nachricht></ListNachricht></Paket>";
}
// clang-format on

EventType get_type(std::vector<ris_message> const& messages) {
  auto content = GetMessage(messages[0].data())->content();
  auto delay_message = reinterpret_cast<DelayMessage const*>(content);
  return delay_message->events()->Get(0)->base()->type();

}

TEST(delay_message, train_event_type) {
  auto start_msg = type_fixture("Start");
  auto start = parse_xmls(pack(start_msg.c_str()));
  ASSERT_EQ(EventType_Departure, get_type(start));

  auto ab_msg = type_fixture("Ab");
  auto ab = parse_xmls(pack(ab_msg.c_str()));
  ASSERT_EQ(EventType_Departure, get_type(ab));

  auto an_msg = type_fixture("An");
  auto an = parse_xmls(pack(an_msg.c_str()));
  ASSERT_EQ(EventType_Arrival, get_type(an));

  auto ziel_msg = type_fixture("Ziel");
  auto ziel = parse_xmls(pack(ziel_msg.c_str()));
  ASSERT_EQ(EventType_Arrival, get_type(ziel));

  // "Durch" events are ignored
  auto pass_msg = type_fixture("Durch");
  auto pass = parse_xmls(pack(pass_msg.c_str()));
  auto content = GetMessage(pass[0].data())->content();
  auto delay_message = reinterpret_cast<DelayMessage const*>(content);
  EXPECT_EQ(0, delay_message->events()->size());
}

// clang-format off
char const* ist_prog_fixture_1 = "<?xml version=\"1.0\" encoding=\"iso-8859-1\" ?>\
<Paket Version=\"1.2\" SpezVer=\"1\" TOut=\"20151006235943156\" KNr=\"123863771\">\
<ListNachricht><Nachricht><IstProg >\
<Service Id=\"85825746\" IdZNr=\"21839\" IdZGattung=\"RE\" IdBf=\"AL\"\
 IdBfEvaNr=\"8000237\" IdZeit=\"20151006232300\"  ZielBfCode=\"ABCH\"\
 ZielBfEvaNr=\"8000058\" Zielzeit=\"20151007000500\" IdVerwaltung=\"02\"\
 IdZGattungInt=\"RE\" IdLinie=\"\" SourceZNr=\"EFZ\">\
<ListZug><Zug Nr=\"21839\" Gattung=\"RE\"  GattungInt=\"RE\" Verwaltung=\"02\" >\
<ListZE><ZE Typ=\"Ziel\" >\
<Bf Code=\"ABCH\" EvaNr=\"8000058\" Name=\"B�chen\"/>\
<Zeit Soll=\"20151007000500\" Prog=\"20151007000600\" Dispo=\"NEIN\" />\
</ZE>    </ListZE></Zug></ListZug></Service></IstProg>\
<ListQuelle><Quelle Sender=\"ZENTRAL\"  Typ=\"IstProg\" KNr=\"18767\"\
 TIn=\"20151006235939097\" TOutSnd=\"20151006235943107\"/>\
<Quelle Sender=\"10.35.204.12:7213/13\" Typ=\"UIC 102\" TIn=\"20151006235942\"\
 Esc=\"bln810jye7\" /></ListQuelle></Nachricht></ListNachricht></Paket>";
// clang-format on

TEST(delay_message, ist_prog_message_1) {
  auto const messages = parse_xmls(pack(ist_prog_fixture_1));
  ASSERT_EQ(1, messages.size());

  auto const& message = messages[0];
  EXPECT_EQ(1444168783, message.timestamp);
  EXPECT_EQ(1444169100, message.scheduled);

  auto outer_msg = GetMessage(message.data());
  ASSERT_EQ(MessageUnion_DelayMessage, outer_msg->content_type());
  auto inner_msg = reinterpret_cast<DelayMessage const*>(outer_msg->content());

  EXPECT_EQ(DelayType_Forecast, inner_msg->type());

  auto events = inner_msg->events();
  ASSERT_EQ(1, events->size());

  EXPECT_EQ(21839, events->Get(0)->base()->trainIndex());
  EXPECT_EQ(StationIdType_EVA, events->Get(0)->base()->stationIdType());
  EXPECT_EQ(std::string("8000058"),
            events->Get(0)->base()->stationId()->c_str());
  EXPECT_EQ(1444169100, events->Get(0)->base()->scheduledTime());
  EXPECT_EQ(EventType_Arrival, events->Get(0)->base()->type());

  EXPECT_EQ(1444169160, events->Get(0)->updatedTime());
}

// clang-format off
char const* ist_prog_fixture_2 = "<?xml version=\"1.0\" encoding=\"iso-8859-1\" ?>\
<Paket Version=\"1.2\" SpezVer=\"1\" TOut=\"20151007000000657\" KNr=\"123863949\">\
<ListNachricht><Nachricht><IstProg >\
<Service Id=\"85712577\" IdZNr=\"37616\" IdZGattung=\"S\" IdBf=\"APB\"\
 IdBfEvaNr=\"8004862\" IdZeit=\"20151006231000\"  ZielBfCode=\"AWL\"\
 ZielBfEvaNr=\"8006236\" Zielzeit=\"20151007001900\" IdVerwaltung=\"0S\"\
 IdZGattungInt=\"s\" IdLinie=\"1\" SourceZNr=\"EFZ\">\
<ListZug><Zug Nr=\"37616\" Gattung=\"S\" Linie=\"1\"  GattungInt=\"s\" Verwaltung=\"0S\" >\
<ListZE>\
<ZE Typ=\"An\" ><Bf Code=\"ARI\" EvaNr=\"8005106\" Name=\"Hamburg-Rissen\"/>\
<Zeit Soll=\"20151007001400\" Prog=\"20151007001800\" Dispo=\"NEIN\" /></ZE>\
<ZE Typ=\"Ab\" ><Bf Code=\"ARI\" EvaNr=\"8005106\" Name=\"Hamburg-Rissen\"/>\
<Zeit Soll=\"20151007001400\" Prog=\"20151007001800\" Dispo=\"NEIN\" /></ZE>\
<ZE Typ=\"Ziel\" ><Bf Code=\"AWL\" EvaNr=\"8006236\" Name=\"Wedel(Holst)\"/>\
<Zeit Soll=\"20151007001900\" Prog=\"20151007002200\" Dispo=\"NEIN\" /></ZE>\
    </ListZE></Zug></ListZug></Service></IstProg>\
<ListQuelle><Quelle Sender=\"SBahnHamburg\"  Typ=\"IstProg\" KNr=\"8909333\"\
 TIn=\"20151006235950236\" TOutSnd=\"20151007000000064\"/></ListQuelle>\
 </Nachricht></ListNachricht></Paket>";
// clang-format on

TEST(delay_message, ist_prog_message_2) {
  auto const messages = parse_xmls(pack(ist_prog_fixture_2));
  ASSERT_EQ(1, messages.size());

  auto const& message = messages[0];
  EXPECT_EQ(1444168800, message.timestamp);
  EXPECT_EQ(1444169940, message.scheduled);

  auto outer_msg = GetMessage(message.data());
  ASSERT_EQ(MessageUnion_DelayMessage, outer_msg->content_type());
  auto inner_msg = reinterpret_cast<DelayMessage const*>(outer_msg->content());

  EXPECT_EQ(DelayType_Forecast, inner_msg->type());

  auto events = inner_msg->events();
  ASSERT_EQ(3, events->size());

  // evt 0
  EXPECT_EQ(37616, events->Get(0)->base()->trainIndex());
  EXPECT_EQ(StationIdType_EVA, events->Get(0)->base()->stationIdType());
  EXPECT_EQ(std::string("8005106"),
            events->Get(0)->base()->stationId()->c_str());
  EXPECT_EQ(1444169640, events->Get(0)->base()->scheduledTime());
  EXPECT_EQ(EventType_Arrival, events->Get(0)->base()->type());

  EXPECT_EQ(1444169880, events->Get(0)->updatedTime());

  // evt 1
  EXPECT_EQ(37616, events->Get(1)->base()->trainIndex());
  EXPECT_EQ(StationIdType_EVA, events->Get(1)->base()->stationIdType());
  EXPECT_EQ(std::string("8005106"),
            events->Get(1)->base()->stationId()->c_str());
  EXPECT_EQ(1444169640, events->Get(1)->base()->scheduledTime());
  EXPECT_EQ(EventType_Departure, events->Get(1)->base()->type());

  EXPECT_EQ(1444169880, events->Get(1)->updatedTime());

  // evt 2
  EXPECT_EQ(37616, events->Get(2)->base()->trainIndex());
  EXPECT_EQ(StationIdType_EVA, events->Get(2)->base()->stationIdType());
  EXPECT_EQ(std::string("8006236"),
            events->Get(2)->base()->stationId()->c_str());
  EXPECT_EQ(1444169940, events->Get(2)->base()->scheduledTime());
  EXPECT_EQ(EventType_Arrival, events->Get(2)->base()->type());

  EXPECT_EQ(1444170120, events->Get(2)->updatedTime());
}

}  // namespace ris
}  // namespace motis
