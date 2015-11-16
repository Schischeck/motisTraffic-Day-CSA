#include "gtest/gtest.h"

#include "include/helper.h"

#include "motis/protocol/RISMessage_generated.h"
#include "motis/ris/risml_parser.h"

namespace motis {
namespace ris {

// clang-format off
char const* cancel_fixture_1 = "<?xml version=\"1.0\" encoding=\"iso-8859-1\" ?>\
<Paket Version=\"1.2\" SpezVer=\"1\" TOut=\"20151007070635992\" KNr=\"124264761\">\
 <ListNachricht>\
  <Nachricht>\
   <Ausfall>\
    <Service Id=\"85721079\" IdZNr=\"99655\" IdZGattung=\"Bus\" IdBf=\"CCTVI\" \
IdBfEvaNr=\"0460711\" IdZeit=\"20151007073000\" ZielBfCode=\"CBESC\" \
ZielBfEvaNr=\"0683407\" Zielzeit=\"20151007075700\" IdVerwaltung=\"ovfNBG\" \
IdZGattungInt=\"Bus\" IdLinie=\"818\" SourceZNr=\"EFZ\">\
     <ListZug>\
      <Zug Nr=\"99655\" Gattung=\"Bus\" Linie=\"818\" GattungInt=\"Bus\" \
Verwaltung=\"ovfNBG\">\
       <ListZE>\
        <ZE Typ=\"An\" Status=\"Ausf\">\
         <Bf Code=\"CDMJP\" EvaNr=\"0680414\" \
Name=\"Bensenstr., Rothenburg ob der Taube\"/>\
         <Zeit Soll=\"20151007075400\"/>\
         <Gleis Soll=\"\"/>\
        </ZE>\
        <ZE Typ=\"Ab\" Status=\"Ausf\">\
         <Bf Code=\"CDMJP\" EvaNr=\"0680414\" \
Name=\"Bensenstr., Rothenburg ob der Taube\"/>\
         <Zeit Soll=\"20151007075400\"/>\
         <Gleis Soll=\"\"/>\
        </ZE>\
       </ListZE>\
      </Zug>\
     </ListZug>\
    </Service>\
   </Ausfall>\
   <ListQuelle>\
    <Quelle Sender=\"RSL\" Typ=\"Ausfall\" KNr=\"64205100707004800093\" \
TIn=\"20151007070635948\" TOutSnd=\"20151007070050\"/>\
   </ListQuelle>\
  </Nachricht>\
 </ListNachricht>\
</Paket>";
// clang-format on

TEST(ris_cancel_message, message_1) {
  auto const messages = parse_xmls(pack(cancel_fixture_1));
  ASSERT_EQ(1, messages.size());

  auto const& message = messages[0];
  EXPECT_EQ(1444194395, message.timestamp);
  EXPECT_EQ(1444197420, message.scheduled);

  auto outer_msg = GetMessage(message.data());
  ASSERT_EQ(MessageUnion_CancelMessage, outer_msg->content_type());
  auto inner_msg = reinterpret_cast<CancelMessage const*>(outer_msg->content());

  auto events = inner_msg->events();
  ASSERT_EQ(2, events->size());

  // evt 0
  EXPECT_EQ(99655, events->Get(0)->trainIndex());
  EXPECT_EQ(StationIdType_EVA, events->Get(0)->stationIdType());
  EXPECT_EQ(std::string("0680414"), events->Get(0)->stationId()->c_str());
  EXPECT_EQ(1444197240, events->Get(0)->scheduledTime());
  EXPECT_EQ(EventType_Arrival, events->Get(0)->type());

  // evt 1
  EXPECT_EQ(99655, events->Get(1)->trainIndex());
  EXPECT_EQ(StationIdType_EVA, events->Get(1)->stationIdType());
  EXPECT_EQ(std::string("0680414"), events->Get(1)->stationId()->c_str());
  EXPECT_EQ(1444197240, events->Get(1)->scheduledTime());
  EXPECT_EQ(EventType_Departure, events->Get(1)->type());
}

// clang-format off
char const* cancel_fixture_2 = "<?xml version=\"1.0\" encoding=\"iso-8859-1\" ?>\
<Paket Version=\"1.2\" SpezVer=\"1\" TOut=\"20151007161500382\" KNr=\"125683842\">\
 <ListNachricht>\
  <Nachricht>\
   <Ausfall>\
    <Service Id=\"86318167\" IdZNr=\"31126\" IdZGattung=\"Bus\" IdBf=\"CCJBN\" \
IdBfEvaNr=\"0732798\" IdZeit=\"20151007155500\" ZielBfCode=\"CCHOU\" \
ZielBfEvaNr=\"0730993\" Zielzeit=\"20151007163600\" IdVerwaltung=\"vbbBVB\" \
IdZGattungInt=\"Bus\" IdLinie=\"M21\" SourceZNr=\"EFZ\">\
     <ListZug>\
      <Zug Nr=\"31126\" Gattung=\"Bus\" Linie=\"M21\" GattungInt=\"Bus\" Verwaltung=\"vbbBVB\">\
       <ListZE>\
        <ZE Typ=\"An\" Status=\"Ausf\">\
         <Bf Code=\"CCHOM\" EvaNr=\"0730985\" Name=\"Jungfernheide Bhf (S+U), Berlin\"/>\
         <Zeit Soll=\"20151007163500\"/>\
         <Gleis Soll=\"\"/>\
        </ZE>\
        <ZE Typ=\"Ab\" Status=\"Ausf\">\
         <Bf Code=\"CCHOM\" EvaNr=\"0730985\" Name=\"Jungfernheide Bhf (S+U), Berlin\"/>\
         <Zeit Soll=\"20151007163500\"/>\
         <Gleis Soll=\"\"/>\
        </ZE>\
        <ZE Typ=\"Ziel\" Status=\"Ausf\">\
         <Bf Code=\"CCHOU\" EvaNr=\"0730993\" Name=\"Goerdelersteg, Berlin\"/>\
         <Zeit Soll=\"20151007163600\"/>\
         <Gleis Soll=\"\"/>\
        </ZE>\
       </ListZE>\
      </Zug>\
     </ListZug>\
    </Service>\
   </Ausfall>\
   <ListQuelle>\
    <Quelle Sender=\"RSL\" Typ=\"Ausfall\" KNr=\"11805100716140800086\" \
TIn=\"20151007161500043\" TOutSnd=\"20151007161409\"/>\
   </ListQuelle>\
  </Nachricht>\
 </ListNachricht>\
</Paket>";
// clang-format on

// TODO!
TEST(ris_ausfall_message, message_2) {
  auto const messages = parse_xmls(pack(cancel_fixture_2));
  ASSERT_EQ(1, messages.size());

  auto const& message = messages[0];
  EXPECT_EQ(1444227300, message.timestamp);
  EXPECT_EQ(1444228560, message.scheduled);

  auto outer_msg = GetMessage(message.data());
  ASSERT_EQ(MessageUnion_CancelMessage, outer_msg->content_type());
  auto inner_msg = reinterpret_cast<CancelMessage const*>(outer_msg->content());

  auto events = inner_msg->events();
  ASSERT_EQ(3, events->size());

  // evt 0
  EXPECT_EQ(31126, events->Get(0)->trainIndex());
  EXPECT_EQ(StationIdType_EVA, events->Get(0)->stationIdType());
  EXPECT_EQ(std::string("0730985"), events->Get(0)->stationId()->c_str());
  EXPECT_EQ(1444228500, events->Get(0)->scheduledTime());
  EXPECT_EQ(EventType_Arrival, events->Get(0)->type());

  // evt 1
  EXPECT_EQ(31126, events->Get(1)->trainIndex());
  EXPECT_EQ(StationIdType_EVA, events->Get(1)->stationIdType());
  EXPECT_EQ(std::string("0730985"), events->Get(1)->stationId()->c_str());
  EXPECT_EQ(1444228500, events->Get(1)->scheduledTime());
  EXPECT_EQ(EventType_Departure, events->Get(1)->type());

  // evt 2
  EXPECT_EQ(31126, events->Get(2)->trainIndex());
  EXPECT_EQ(StationIdType_EVA, events->Get(2)->stationIdType());
  EXPECT_EQ(std::string("0730993"), events->Get(2)->stationId()->c_str());
  EXPECT_EQ(1444228560, events->Get(2)->scheduledTime());
  EXPECT_EQ(EventType_Arrival, events->Get(2)->type());
}

}  // namespace ris
}  // namespace motis
