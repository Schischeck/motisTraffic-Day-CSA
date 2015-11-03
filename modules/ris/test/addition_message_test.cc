#include "gtest/gtest.h"

#include "include/helper.h"

#include "motis/protocol/RISMessage_generated.h"
#include "motis/ris/risml_parser.h"

namespace motis {
namespace ris {

// clang-format off
char const* addition_fixture_1 = "<?xml version=\"1.0\" encoding=\"iso-8859-1\" ?>\
 <Paket Version=\"1.2\" SpezVer=\"1\" TOut=\"20151007001025011\" KNr=\"123869635\">\
  <ListNachricht>\
   <Nachricht>\
    <Zusatzzug>\
     <Service Id=\"-787135\" IdZNr=\"2941\" IdZGattung=\"IC\" IdBf=\"NPA\" \
IdBfEvaNr=\"8000298\" IdZeit=\"20151007102000\" RegSta=\"Sonderzug\" \
ZielBfCode=\"KFKB\" ZielBfEvaNr=\"8003330\" Zielzeit=\"20151007182500\" \
IdVerwaltung=\"80\" IdZGattungInt=\"IRX\" SourceZNr=\"EFZ\">\
      <ListZug>\
       <Zug Nr=\"2941\" Gattung=\"IC\" GattungInt=\"IRX\" Verwaltung=\"80\">\
        <ListZE>\
         <ZE Typ=\"Start\">\
          <Bf Code=\"NPA\" EvaNr=\"8000298\" Name=\"Passau Hbf\"/>\
          <Zeit Soll=\"20151007102000\"/>\
         </ZE>\
         <ZE Typ=\"Ziel\">\
          <Bf Code=\"KFKB\" EvaNr=\"8003330\" Name=\"Koln/Bonn Flughafen\"/>\
          <Zeit Soll=\"20151007182500\"/>\
         </ZE>\
        </ListZE>\
       </Zug>\
      </ListZug>\
      <ListZusZug/>\
     </Service>\
    </Zusatzzug>\
    <ListQuelle>\
     <Quelle Sender=\"ZENTRAL\" Typ=\"Zusatzzug\" KNr=\"19178\" \
TIn=\"20151007001015680\" TOutSnd=\"20151007001023691\"/>\
    </ListQuelle>\
   </Nachricht>\
  </ListNachricht>\
 </Paket>";
// clang-format on

TEST(addition_message, message_1) {
  auto const msg = parse_xmls(pack(addition_fixture_1));
  auto const batch = msg->content<RISBatch const*>();

  EXPECT_EQ(1444169425, batch->packets()->Get(0)->timestamp());

  ASSERT_EQ(1, batch->packets()->size());
  ASSERT_EQ(1, batch->packets()->Get(0)->messages()->size());

  auto outer_msg = batch->packets()->Get(0)->messages()->Get(0);
  ASSERT_EQ(MessageUnion_AdditionMessage, outer_msg->content_type());
  auto inner_msg =
      reinterpret_cast<AdditionMessage const*>(outer_msg->content());

  auto events = inner_msg->events();
  ASSERT_EQ(2, events->size());

  auto e0 = events->Get(0);
  EXPECT_EQ(2941, e0->base()->trainIndex());
  EXPECT_EQ(StationIdType_EVA, e0->base()->stationIdType());
  EXPECT_EQ(std::string("8000298"), e0->base()->stationId()->c_str());
  EXPECT_EQ(1444206000, e0->base()->scheduledTime());
  EXPECT_EQ(EventType_Departure, e0->base()->type());

  EXPECT_EQ(std::string("IC"), e0->trainCategory()->c_str());
  EXPECT_EQ(nullptr, e0->track());

  auto e1 = events->Get(1);
  EXPECT_EQ(2941, e1->base()->trainIndex());
  EXPECT_EQ(StationIdType_EVA, e1->base()->stationIdType());
  EXPECT_EQ(std::string("8003330"), e1->base()->stationId()->c_str());
  EXPECT_EQ(1444235100, e1->base()->scheduledTime());
  EXPECT_EQ(EventType_Arrival, e1->base()->type());

  EXPECT_EQ(std::string("IC"), e1->trainCategory()->c_str());
  EXPECT_EQ(nullptr, e1->track());
}

// clang-format off
char const* addition_fixture_2 = "<?xml version=\"1.0\" encoding=\"iso-8859-1\" ?>\
 <Paket Version=\"1.2\" SpezVer=\"1\" TOut=\"20151007043812499\" KNr=\"123928646\">\
  <ListNachricht>\
   <Nachricht>\
    <Zusatzzug>\
     <Service Id=\"-787137\" IdZNr=\"2570\" IdZGattung=\"EC\" IdBf=\"MH\" \
IdBfEvaNr=\"8000261\" IdZeit=\"20151008144800\" RegSta=\"Ersatzzug\" ZielBfCode=\"TS\" \
ZielBfEvaNr=\"8000096\" Zielzeit=\"20151008170400\" IdVerwaltung=\"80\" \
IdZGattungInt=\"ECW\" SourceZNr=\"EFZ\">\
      <ListZug>\
       <Zug Nr=\"2570\" Gattung=\"EC\" GattungInt=\"ECW\" Verwaltung=\"80\">\
        <ListZE>\
         <ZE Typ=\"Start\">\
          <Bf Code=\"MH\" EvaNr=\"8000261\" Name=\"Munchen Hbf\"/>\
          <Zeit Soll=\"20151008144800\"/>\
          <Gleis Soll=\"19\"/>\
         </ZE>\
         <ZE Typ=\"An\">\
          <Bf Code=\"MP\" EvaNr=\"8004158\" Name=\"Munchen-Pasing\"/>\
          <Zeit Soll=\"20151008145400\"/>\
         </ZE>\
         <ZE Typ=\"Ab\">\
          <Bf Code=\"MP\" EvaNr=\"8004158\" Name=\"Munchen-Pasing\"/>\
          <Zeit Soll=\"20151008145600\"/>\
         </ZE>\
         <ZE Typ=\"Ziel\">\
          <Bf Code=\"TS\" EvaNr=\"8000096\" Name=\"Stuttgart Hbf\"/>\
          <Zeit Soll=\"20151008170400\"/>\
          <Gleis Soll=\"9\"/>\
         </ZE>\
        </ListZE>\
       </Zug>\
      </ListZug>\
      <ListZusZug>\
       <ZusZug Bez=\"Referenzzug\">\
        <Service Id=\"86087470\" IdZNr=\"2362\" IdZGattung=\"IC\" IdBf=\"MH\" \
IdBfEvaNr=\"8000261\" IdZeit=\"20151008144800\" ZielBfCode=\"RK\" ZielBfEvaNr=\"8000191\" \
Zielzeit=\"20151008180100\" IdVerwaltung=\"80\" IdZGattungInt=\"IC\" SourceZNr=\"EFZ\">\
         <ListZug>\
          <Zug Nr=\"2362\"/>\
         </ListZug>\
        </Service>\
       </ZusZug>\
      </ListZusZug>\
     </Service>\
    </Zusatzzug>\
    <ListQuelle>\
     <Quelle Sender=\"ZENTRAL\" Typ=\"Zusatzzug\" KNr=\"26425\" \
TIn=\"20151007043809952\" TOutSnd=\"20151007043811898\"/>\
    </ListQuelle>\
   </Nachricht>\
  </ListNachricht>\
 </Paket>";
// clang-format on

TEST(addition_message, message_2) {
  auto const msg = parse_xmls(pack(addition_fixture_2));
  auto const batch = msg->content<RISBatch const*>();

  EXPECT_EQ(1444185492, batch->packets()->Get(0)->timestamp());

  ASSERT_EQ(1, batch->packets()->size());
  ASSERT_EQ(1, batch->packets()->Get(0)->messages()->size());

  auto outer_msg = batch->packets()->Get(0)->messages()->Get(0);
  ASSERT_EQ(MessageUnion_AdditionMessage, outer_msg->content_type());
  auto inner_msg =
      reinterpret_cast<AdditionMessage const*>(outer_msg->content());

  auto events = inner_msg->events();
  ASSERT_EQ(4, events->size());

  auto e0 = events->Get(0);
  EXPECT_EQ(2570, e0->base()->trainIndex());
  EXPECT_EQ(StationIdType_EVA, e0->base()->stationIdType());
  EXPECT_EQ(std::string("8000261"), e0->base()->stationId()->c_str());
  EXPECT_EQ(1444308480, e0->base()->scheduledTime());
  EXPECT_EQ(EventType_Departure, e0->base()->type());

  EXPECT_EQ(std::string("EC"), e0->trainCategory()->c_str());
  EXPECT_EQ(std::string("19"), e0->track()->c_str());

  auto e1 = events->Get(1);
  EXPECT_EQ(2570, e1->base()->trainIndex());
  EXPECT_EQ(StationIdType_EVA, e1->base()->stationIdType());
  EXPECT_EQ(std::string("8004158"), e1->base()->stationId()->c_str());
  EXPECT_EQ(1444308840, e1->base()->scheduledTime());
  EXPECT_EQ(EventType_Arrival, e1->base()->type());

  EXPECT_EQ(std::string("EC"), e1->trainCategory()->c_str());
  EXPECT_EQ(nullptr, e1->track());

  auto e2 = events->Get(2);
  EXPECT_EQ(2570, e2->base()->trainIndex());
  EXPECT_EQ(StationIdType_EVA, e2->base()->stationIdType());
  EXPECT_EQ(std::string("8004158"), e2->base()->stationId()->c_str());
  EXPECT_EQ(1444308960, e2->base()->scheduledTime());
  EXPECT_EQ(EventType_Departure, e2->base()->type());

  EXPECT_EQ(std::string("EC"), e2->trainCategory()->c_str());
  EXPECT_EQ(nullptr, e2->track());

  auto e3 = events->Get(3);
  EXPECT_EQ(2570, e3->base()->trainIndex());
  EXPECT_EQ(StationIdType_EVA, e3->base()->stationIdType());
  EXPECT_EQ(std::string("8000096"), e3->base()->stationId()->c_str());
  EXPECT_EQ(1444316640, e3->base()->scheduledTime());
  EXPECT_EQ(EventType_Arrival, e3->base()->type());

  EXPECT_EQ(std::string("EC"), e3->trainCategory()->c_str());
  EXPECT_EQ(std::string("9"), e3->track()->c_str());
}

}  // namespace ris
}  // namespace motis
