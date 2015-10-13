#include "gtest/gtest.h"

#include <iostream>

#include "flatbuffers/flatbuffers.h"

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
  auto const msg = parse_xmls({ist_fixture_1});
  auto const batch = msg->content<RISBatch const*>();

  EXPECT_EQ(1444168774, batch->packets()->Get(0)->timestamp());

  ASSERT_EQ(1, batch->packets()->size());
  ASSERT_EQ(1, batch->packets()->Get(0)->messages()->size());

  auto outer_msg = batch->packets()->Get(0)->messages()->Get(0);
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
<ListZE><ZE Typ=\"Durch\" ><Bf Code=\"MNFG\" />\
<Zeit Soll=\"20151006234800\" Ist=\"20151006235900\" />\
</ZE>    </ListZE></Zug></ListZug></Service></Ist><ListQuelle>\
<Quelle Sender=\"ZENTRAL\"  Typ=\"IstProg\" KNr=\"18777\" TIn=\"20151007000000336\"\
 TOutSnd=\"20151007000002053\"/><Quelle Sender=\"10.35.205.140:7213/13\"\
 Typ=\"UIC 102\" TIn=\"20151007000001\" Esc=\"mue810jyhi\" />\
</ListQuelle></Nachricht></ListNachricht></Paket>";
// clang-format on

TEST(delay_message, ist_message_2) {
  auto const msg = parse_xmls({ist_fixture_2});
  auto const batch = msg->content<RISBatch const*>();

  EXPECT_EQ(1444168802, batch->packets()->Get(0)->timestamp());

  ASSERT_EQ(1, batch->packets()->size());
  ASSERT_EQ(1, batch->packets()->Get(0)->messages()->size());

  auto outer_msg = batch->packets()->Get(0)->messages()->Get(0);
  ASSERT_EQ(MessageUnion_DelayMessage, outer_msg->content_type());
  auto inner_msg = reinterpret_cast<DelayMessage const*>(outer_msg->content());

  EXPECT_EQ(DelayType_Is, inner_msg->type());

  auto events = inner_msg->events();
  ASSERT_EQ(1, events->size());

  EXPECT_EQ(60418, events->Get(0)->base()->trainIndex());
  EXPECT_EQ(StationIdType_DS100, events->Get(0)->base()->stationIdType());
  EXPECT_EQ(std::string("MNFG"), events->Get(0)->base()->stationId()->c_str());
  EXPECT_EQ(1444168080, events->Get(0)->base()->scheduledTime());
  EXPECT_EQ(EventType_Pass, events->Get(0)->base()->type());

  EXPECT_EQ(1444168740, events->Get(0)->updatedTime());
}

// clang-format off
std::string type_fixture(std::string type_string) {
  return std::string("<?xml version=\"1.0\" encoding=\"iso-8859-1\" ?>\
<Paket><ListNachricht><Nachricht><Ist><Service>\
<ListZug><Zug><ListZE><ZE Typ=\"") + type_string + "\" >\
<Bf/><Zeit/></ZE></ListZE></Zug></ListZug></Service></Ist><ListQuelle>\
</ListQuelle></Nachricht></ListNachricht></Paket>";
}
// clang-format on

EventType get_type(motis::module::msg_ptr const& msg) {
  auto batch = msg->content<RISBatch const*>();
  auto content = batch->packets()->Get(0)->messages()->Get(0)->content();
  auto delay_message = reinterpret_cast<DelayMessage const*>(content);
  return delay_message->events()->Get(0)->base()->type();
}

TEST(delay_message, train_event_type) {
  auto start_msg = type_fixture("Start");
  auto start = parse_xmls({start_msg.c_str()});
  ASSERT_EQ(EventType_Departure, get_type(start));

  auto ab_msg = type_fixture("Ab");
  auto ab = parse_xmls({ab_msg.c_str()});
  ASSERT_EQ(EventType_Departure, get_type(ab));

  auto pass_msg = type_fixture("Durch");
  auto pass = parse_xmls({pass_msg.c_str()});
  ASSERT_EQ(EventType_Pass, get_type(pass));

  auto an_msg = type_fixture("An");
  auto an = parse_xmls({an_msg.c_str()});
  ASSERT_EQ(EventType_Arrival, get_type(an));

  auto ziel_msg = type_fixture("Ziel");
  auto ziel = parse_xmls({ziel_msg.c_str()});
  ASSERT_EQ(EventType_Arrival, get_type(ziel));
}

}  // namespace ris
}  // namespace motis
