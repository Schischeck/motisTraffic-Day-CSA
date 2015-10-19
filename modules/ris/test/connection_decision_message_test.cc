#include "gtest/gtest.h"

#include "include/helper.h"

#include "motis/protocol/RISMessage_generated.h"
#include "motis/ris/risml_parser.h"

namespace motis {
namespace ris {

// clang-format off
char const* connection_decision_fixture_1 = "<?xml version=\"1.0\" encoding=\"iso-8859-1\" ?>\
<Paket Version=\"1.2\" SpezVer=\"1\" TOut=\"20151007161458249\" KNr=\"125683791\">\
 <ListNachricht>\
  <Nachricht>\
   <Anschluss>\
    <ZE Typ=\"An\">\
     <Zug Nr=\"75\" Gattung=\"ICE\" GattungInt=\"ICE\" Verwaltung=\"80\"/>\
     <Bf Code=\"RF\" EvaNr=\"8000107\" Name=\"Freiburg(Breisgau) Hbf\"/>\
     <Zeit Soll=\"20151007161000\" Prog=\"20151007164900\"/>\
     <Service Id=\"85943497\" IdZNr=\"75\" IdZGattung=\"ICE\" IdBf=\"AA\" \
IdBfEvaNr=\"8002553\" IdZeit=\"20151007100900\" ZielBfCode=\"XSZH\" \
ZielBfEvaNr=\"8503000\" Zielzeit=\"20151007180000\" IdVerwaltung=\"80\" \
IdZGattungInt=\"ICE\" SourceZNr=\"EFZ\"/>\
     <ListAnschl>\
      <Anschl Status=\"NichtGehalten\">\
       <ZE Typ=\"Start\">\
        <Zug Nr=\"87488\" Gattung=\"IRE\" GattungInt=\"IRE\" Verwaltung=\"06\"/>\
        <Zeit Soll=\"20151007164500\"/>\
        <Service Id=\"86039657\" IdZNr=\"87488\" IdZGattung=\"IRE\" IdBf=\"RF\" \
IdBfEvaNr=\"8000107\" IdZeit=\"20151007164500\" ZielBfCode=\"XFMV\" \
ZielBfEvaNr=\"8700031\" Zielzeit=\"20151007173000\" IdVerwaltung=\"06\" \
IdZGattungInt=\"IRE\" SourceZNr=\"EFZ\"/>\
        <ListAltAnschl/>\
       </ZE>\
      </Anschl>\
     </ListAnschl>\
    </ZE>\
   </Anschluss>\
   <ListQuelle>\
    <Quelle Sender=\"ZENTRAL\" Typ=\"Anschluss\" KNr=\"19378\" TIn=\"20151007161456317\" TOutSnd=\"20151007161457915\"/>\
   </ListQuelle>\
  </Nachricht>\
 </ListNachricht>\
</Paket>";
// clang-format on

TEST(connection_decision_message, message_1) {
  auto const msg = parse_xmls(pack(connection_decision_fixture_1));
  auto const batch = msg->content<RISBatch const*>();

  EXPECT_EQ(1444227298, batch->packets()->Get(0)->timestamp());

  ASSERT_EQ(1, batch->packets()->size());
  ASSERT_EQ(1, batch->packets()->Get(0)->messages()->size());

  auto outer_msg = batch->packets()->Get(0)->messages()->Get(0);
  ASSERT_EQ(MessageUnion_ConnectionDecisionMessage, outer_msg->content_type());
  auto inner_msg = reinterpret_cast<ConnectionDecisionMessage const*>(outer_msg->content());

  auto from = inner_msg->from();
  EXPECT_EQ(75, from->trainIndex());
  EXPECT_EQ(StationIdType_EVA, from->stationIdType());
  EXPECT_EQ(std::string("8000107"), from->stationId()->c_str());
  EXPECT_EQ(1444227000, from->scheduledTime());
  EXPECT_EQ(EventType_Arrival, from->type());

  auto to = inner_msg->to();
  ASSERT_EQ(1, to->size());

  auto e0 = to->Get(0);
  EXPECT_EQ(87488, e0->base()->trainIndex());
  EXPECT_EQ(StationIdType_Context, e0->base()->stationIdType());
  EXPECT_EQ(nullptr, e0->base()->stationId());
  EXPECT_EQ(1444229100, e0->base()->scheduledTime());
  EXPECT_EQ(EventType_Departure, e0->base()->type());

  EXPECT_EQ(false, e0->hold());
}


// clang-format off
char const* connection_decision_fixture_2 = "<?xml version=\"1.0\" encoding=\"iso-8859-1\" ?>\
<Paket Version=\"1.2\" SpezVer=\"1\" TOut=\"20151007161453864\" KNr=\"125683640\">\
 <ListNachricht>\
  <Nachricht>\
   <Anschluss>\
    <ZE Typ=\"An\">\
     <Zug Nr=\"1004\" Gattung=\"ICE\" GattungInt=\"ICE\" Verwaltung=\"80\"/>\
     <Bf Code=\"NN\" EvaNr=\"8000284\" Name=\"N<FC>\rnberg Hbf\"/>\
     <Zeit Soll=\"20151007162800\" Prog=\"20151007163000\"/>\
     <Service Id=\"85904967\" IdZNr=\"1004\" IdZGattung=\"ICE\" IdBf=\"MH\" \
IdBfEvaNr=\"8000261\" IdZeit=\"20151007151600\" ZielBfCode=\"BGS\" \
ZielBfEvaNr=\"8011102\" Zielzeit=\"20151007212700\" IdVerwaltung=\"80\" \
IdZGattungInt=\"ICE\" SourceZNr=\"EFZ\"/>\
     <ListAnschl>\
      <Anschl Status=\"Gehalten\">\
       <ZE Typ=\"Ab\">\
        <Zug Nr=\"584\" Gattung=\"ICE\" GattungInt=\"ICE\" Verwaltung=\"80\"/>\
        <Zeit Soll=\"20151007163400\"/>\
        <Service Id=\"85691162\" IdZNr=\"584\" IdZGattung=\"ICE\" IdBf=\"MH\" \
IdBfEvaNr=\"8000261\" IdZeit=\"20151007152000\" ZielBfCode=\"AL\" \
ZielBfEvaNr=\"8000237\" Zielzeit=\"20151007214200\" IdVerwaltung=\"80\" \
IdZGattungInt=\"ICE\" SourceZNr=\"EFZ\"/>\
        <ListAltAnschl/>\
       </ZE>\
      </Anschl>\
     </ListAnschl>\
    </ZE>\
   </Anschluss>\
   <ListQuelle>\
    <Quelle Sender=\"ZENTRAL\" Typ=\"Anschluss\" KNr=\"19373\" \
TIn=\"20151007161451761\" TOutSnd=\"20151007161453636\"/>\
   </ListQuelle>\
  </Nachricht>\
 </ListNachricht>\
</Paket>";
// clang-format on

TEST(connection_decision_message, message_2) {
  auto const msg = parse_xmls(pack(connection_decision_fixture_2));
  auto const batch = msg->content<RISBatch const*>();

  EXPECT_EQ(1444227293, batch->packets()->Get(0)->timestamp());

  ASSERT_EQ(1, batch->packets()->size());
  ASSERT_EQ(1, batch->packets()->Get(0)->messages()->size());

  auto outer_msg = batch->packets()->Get(0)->messages()->Get(0);
  ASSERT_EQ(MessageUnion_ConnectionDecisionMessage, outer_msg->content_type());
  auto inner_msg = reinterpret_cast<ConnectionDecisionMessage const*>(outer_msg->content());

  auto from = inner_msg->from();
  EXPECT_EQ(1004, from->trainIndex());
  EXPECT_EQ(StationIdType_EVA, from->stationIdType());
  EXPECT_EQ(std::string("8000284"), from->stationId()->c_str());
  EXPECT_EQ(1444228080, from->scheduledTime());
  EXPECT_EQ(EventType_Arrival, from->type());

  auto to = inner_msg->to();
  ASSERT_EQ(1, to->size());

  auto e0 = to->Get(0);
  EXPECT_EQ(584, e0->base()->trainIndex());
  EXPECT_EQ(StationIdType_Context, e0->base()->stationIdType());
  EXPECT_EQ(nullptr, e0->base()->stationId());
  EXPECT_EQ(1444228440, e0->base()->scheduledTime());
  EXPECT_EQ(EventType_Departure, e0->base()->type());

  EXPECT_EQ(true, e0->hold());
}

}  // namespace ris
}  // namespace motis
