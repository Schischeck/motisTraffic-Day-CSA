#include "gtest/gtest.h"

#include "parser/buffer.h"

#include "motis/bikesharing/nextbike_parser.h"

namespace motis {
namespace bikesharing {

char const* xml_fixture = R"((
<?xml version="1.0" encoding="utf-8"?>
<markers>
  <country lat="50.7086" lng="10.6348" zoom="5" name="nextbike Germany"
      hotline="+493069205046" domain="de" country="DE" country_name="Germany"
      terms="http://www.nextbike.de/media/nextbike_AGB_de_en_03-2015.pdf"
      website="http://www.nextbike.de/">
    <city uid="1" lat="51.3415" lng="12.3625" zoom="14" maps_icon=""
        alias="leipzig" break="0" name="Leipzig">
      <place uid="28" lat="51.3405051597014" lng="12.3688137531281"
          name="Gottschedstraße/Bosestraße " spot="1" number="4013" bikes="3"
          terminal_type="unknown" bike_numbers="10042,10657,10512" />
      <place uid="128" lat="51.3371237726003" lng="12.37330377101898"
          name="Burgplatz/Freifläche/Zaun" spot="1" number="4011" bikes="5+" 
          terminal_type="unknown" bike_numbers="10520,10452,10114,10349,10297" />
    </city>
  </country>
</markers>
))";

TEST(bikesharing_nextbike_parser, parser_test) {
  auto result = parse_nextbike_xml(parser::buffer{xml_fixture});

  ASSERT_EQ(2, result.size());

  auto r0 = result[0];
  EXPECT_EQ(28, r0.uid);
  EXPECT_EQ(51.3405051597014, r0.lat);
  EXPECT_EQ(12.3688137531281, r0.lng);
  EXPECT_EQ(std::string{"Gottschedstraße/Bosestraße "}, r0.name);
  EXPECT_EQ(3, r0.available_bikes);

  auto r1 = result[1];
  EXPECT_EQ(128, r1.uid);
  EXPECT_EQ(51.3371237726003, r1.lat);
  EXPECT_EQ(12.37330377101898, r1.lng);
  EXPECT_EQ(std::string{"Burgplatz/Freifläche/Zaun"}, r1.name);
  EXPECT_EQ(5, r1.available_bikes);
}

}  // namespace bikesharing
}  // namespace motis