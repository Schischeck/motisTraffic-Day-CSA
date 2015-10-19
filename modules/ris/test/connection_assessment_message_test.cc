#include "gtest/gtest.h"

#include "include/helper.h"

#include "motis/protocol/RISMessage_generated.h"
#include "motis/ris/risml_parser.h"

namespace motis {
namespace ris {

// clang-format off
char const* connection_assessment_fixture_1 = "<?xml version=\"1.0\" encoding=\"iso-8859-1\" ?>\
<Paket Version=\"1.2\" SpezVer=\"1\" TOut=\"20151007051838050\" KNr=\"123984009\">\
<ListNachricht><Nachricht><Anschlussbewertung>\
  <ZE Typ=\"An\">\
    <Bf EvaNr=\"8001585\"/>\
    <ListAnschl>\
      <Anschl Bewertung=\"2\">\
        <ZE Typ=\"Ab\">\
          <Bf EvaNr=\"8001585\"/>\
          <Service Id=\"85751154\" IdBfEvaNr=\"8004005\" IdTyp=\"Ab\" \
IdVerwaltung=\"M2\" IdZGattung=\"S\" IdZGattungInt=\"DPN\" IdZNr=\"90708\" \
IdZeit=\"20151007052500\" SourceZNR=\"EFZ\" ZielBfEvaNr=\"8000430\" Zielzeit=\"20151007061600\">\
            <ListZug/>\
          </Service>\
          <Zeit Ist=\"\" Prog=\"\" Soll=\"20151007055000\"/>\
          <Zug Gattung=\"S\" GattungInt=\"DPN\" Nr=\"90708\" Verwaltung=\"M2\">\
            <ListZE/>\
          </Zug>\
        </ZE>\
      </Anschl>\
    </ListAnschl>\
    <Service Id=\"86090468\" IdBfEvaNr=\"8000253\" IdTyp=\"An\" IdVerwaltung=\"03\" \
IdZGattung=\"S\" IdZGattungInt=\"s\" IdZNr=\"30815\" IdZeit=\"20151007051400\" \
SourceZNR=\"EFZ\" ZielBfEvaNr=\"8000142\" Zielzeit=\"20151007065800\">\
      <ListZug/>\
    </Service>\
    <Zeit Ist=\"\" Prog=\"\" Soll=\"20151007054400\"/>\
    <Zug Gattung=\"S\" GattungInt=\"s\" Nr=\"30815\" Verwaltung=\"03\">\
      <ListZE/>\
    </Zug>\
  </ZE>\
</Anschlussbewertung>\
<ListQuelle>\
<Quelle Sender=\"RSL\"  Typ=\"Anschlussbewertung\" KNr=\"15100705182500000\"\
 TIn=\"20151007051838039\" TOutSnd=\"20151007051825\"/>\
</ListQuelle></Nachricht></ListNachricht></Paket>";
// clang-format on

TEST(connection_assessment_message, message_1) {
  auto const msg = parse_xmls(pack(connection_assessment_fixture_1));
  auto const batch = msg->content<RISBatch const*>();

  EXPECT_EQ(1444187918, batch->packets()->Get(0)->timestamp());

  ASSERT_EQ(1, batch->packets()->size());
  ASSERT_EQ(1, batch->packets()->Get(0)->messages()->size());

  auto outer_msg = batch->packets()->Get(0)->messages()->Get(0);
  ASSERT_EQ(MessageUnion_ConnectionAssessmentMessage, outer_msg->content_type());
  auto inner_msg = reinterpret_cast<ConnectionAssessmentMessage const*>(outer_msg->content());

  auto from = inner_msg->from();
  EXPECT_EQ(30815, from->trainIndex());
  EXPECT_EQ(StationIdType_EVA, from->stationIdType());
  EXPECT_EQ(std::string("8001585"), from->stationId()->c_str());
  EXPECT_EQ(1444189440, from->scheduledTime());
  EXPECT_EQ(EventType_Arrival, from->type());

  auto to = inner_msg->to();
  ASSERT_EQ(1, to->size());

  auto e0 = to->Get(0);
  EXPECT_EQ(90708, e0->base()->trainIndex());
  EXPECT_EQ(StationIdType_EVA, e0->base()->stationIdType());
  EXPECT_EQ(std::string("8001585"), e0->base()->stationId()->c_str());
  EXPECT_EQ(1444189800, e0->base()->scheduledTime());
  EXPECT_EQ(EventType_Departure, e0->base()->type());

  EXPECT_EQ(2, e0->assessment());
}


// clang-format off
char const* connection_assessment_fixture_2 = "<?xml version=\"1.0\" encoding=\"iso-8859-1\" ?>\
<Paket Version=\"1.2\" SpezVer=\"1\" TOut=\"20151006235948056\" KNr=\"123863851\">\
<ListNachricht>\
<Nachricht>\
<Anschlussbewertung>\
<ZE Typ=\"Ziel\">\
<Bf EvaNr=\"8000261\"/>\
<ListAnschl>\
<Anschl Bewertung=\"4\">\
<ZE Typ=\"Ab\">\
<Bf EvaNr=\"8098263\"/>\
<Service Id=\"85777037\" IdBfEvaNr=\"8002980\" IdTyp=\"Ab\" IdVerwaltung=\"07\"\
 IdZGattung=\"S\" IdZGattungInt=\"s\" IdZNr=\"8326\" IdZeit=\"20151006233600\"\
 SourceZNR=\"EFZ\" ZielBfEvaNr=\"8004204\" Zielzeit=\"20151007005600\">\
<ListZug/>\
</Service>\
<Zeit Ist=\"\" Prog=\"\" Soll=\"20151007001900\"/>\
<Zug Gattung=\"S\" GattungInt=\"s\" Nr=\"8326\" Verwaltung=\"07\">\
<ListZE/>\
</Zug>\
</ZE>\
</Anschl>\
<Anschl Bewertung=\"3\">\
<ZE Typ=\"Ab\">\
<Bf EvaNr=\"8098263\"/>\
<Service Id=\"85967814\" IdBfEvaNr=\"8002347\" IdTyp=\"Ab\" IdVerwaltung=\"07\"\
 IdZGattung=\"S\" IdZGattungInt=\"s\" IdZNr=\"8426\" IdZeit=\"20151006234100\"\
 SourceZNR=\"EFZ\" ZielBfEvaNr=\"8000119\" Zielzeit=\"20151007010500\">\
<ListZug/>\
</Service>\
<Zeit Ist=\"\" Prog=\"\" Soll=\"20151007002100\"/>\
<Zug Gattung=\"S\" GattungInt=\"s\" Nr=\"8426\" Verwaltung=\"07\">\
<ListZE/>\
</Zug>\
</ZE>\
</Anschl>\
</ListAnschl>\
<Service Id=\"86059254\" IdBfEvaNr=\"8004775\" IdTyp=\"Ziel\" IdVerwaltung=\"07\"\
 IdZGattung=\"S\" IdZGattungInt=\"s\" IdZNr=\"8239\" IdZeit=\"20151006233200\"\
 SourceZNR=\"EFZ\" ZielBfEvaNr=\"8000261\" Zielzeit=\"20151007000800\">\
<ListZug/>\
</Service>\
<Zeit Ist=\"\" Prog=\"\" Soll=\"20151007000800\"/>\
<Zug Gattung=\"S\" GattungInt=\"s\" Nr=\"8239\" Verwaltung=\"07\">\
<ListZE/>\
</Zug>\
</ZE>\
</Anschlussbewertung>\
<ListQuelle>\
<Quelle Sender=\"RSL\" Typ=\"Anschlussbewertung\" KNr=\"15100623594700000\"\
 TIn=\"20151006235944623\" TOutSnd=\"20151006235947\"/>\
</ListQuelle>\
</Nachricht>\
</ListNachricht>\
</Paket>";
// clang-format on

TEST(connection_assessment_message, message_2) {
  auto const msg = parse_xmls(pack(connection_assessment_fixture_2));
  auto const batch = msg->content<RISBatch const*>();

  EXPECT_EQ(1444168788, batch->packets()->Get(0)->timestamp());

  ASSERT_EQ(1, batch->packets()->size());
  ASSERT_EQ(1, batch->packets()->Get(0)->messages()->size());

  auto outer_msg = batch->packets()->Get(0)->messages()->Get(0);
  ASSERT_EQ(MessageUnion_ConnectionAssessmentMessage, outer_msg->content_type());
  auto inner_msg = reinterpret_cast<ConnectionAssessmentMessage const*>(outer_msg->content());

  auto from = inner_msg->from();
  EXPECT_EQ(8239, from->trainIndex());
  EXPECT_EQ(StationIdType_EVA, from->stationIdType());
  EXPECT_EQ(std::string("8000261"), from->stationId()->c_str());
  EXPECT_EQ(1444169280, from->scheduledTime());
  EXPECT_EQ(EventType_Arrival, from->type());

  auto to = inner_msg->to();
  ASSERT_EQ(2, to->size());

  auto e0 = to->Get(0);
  EXPECT_EQ(8326, e0->base()->trainIndex());
  EXPECT_EQ(StationIdType_EVA, e0->base()->stationIdType());
  EXPECT_EQ(std::string("8098263"), e0->base()->stationId()->c_str());
  EXPECT_EQ(1444169940, e0->base()->scheduledTime());
  EXPECT_EQ(EventType_Departure, e0->base()->type());

  EXPECT_EQ(4, e0->assessment());

  auto e1 = to->Get(1);
  EXPECT_EQ(8426, e1->base()->trainIndex());
  EXPECT_EQ(StationIdType_EVA, e1->base()->stationIdType());
  EXPECT_EQ(std::string("8098263"), e1->base()->stationId()->c_str());
  EXPECT_EQ(1444170060, e1->base()->scheduledTime());
  EXPECT_EQ(EventType_Departure, e1->base()->type());

  EXPECT_EQ(3, e1->assessment());
}

}  // namespace ris
}  // namespace motis
