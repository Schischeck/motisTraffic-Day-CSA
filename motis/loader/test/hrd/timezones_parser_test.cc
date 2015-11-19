namespace motis {
namespace loader {
namespace hrd {

using namespace boost::filesystem;
using namespace motis::logging;

class loader_hrd_timezones_test : testing::Test {

  virtual void SetUp() {
    data_.emplace_back("zeitvs.101", TIMEZONES_TEST_DATA);
    data_.emplace_back("eckdaten.101", BASIC_DATA_TEST_DATA);
    tz = parse_timezones(data_[0], data_[1]);
  }

private:
  timezones tz;
  std::vector<loaded_file> data_;
  static char const* const TIMEZONES_TEST_DATA = R"(%
0000000 +0100 +0200 29032015 0200 25102015 0300 %  Nahverkehrsdaten; MEZ=GMT+1
1000000 +0200 +0300 29032015 0300 25102015 0400 %  Finnland
2000000 +0300                                   %  Russland
2100000 +0300                                   %  Weißrussland
2200000 +0200 +0300 29032015 0300 25102015 0400 %  Ukraine
2300000 +0200 +0300 29032015 0300 25102015 0400 %  Moldawien
2400000 +0200 +0300 29032015 0300 25102015 0400 %  Litauen
2500000 +0200 +0300 29032015 0300 25102015 0400 %  Lettland
2600000 +0200 +0300 29032015 0300 25102015 0400 %  Estland
2700000 +0600                                   %  Kasachstan
2800000 +0300                                   %  Georgien
2900000 +0500                                   %  Usbekistan
3000000 +0900                                   %  Nordkorea
3100000 +0800                                   %  Mongolei
3300000 +0800                                   %  China
5200000 +0200 +0300 29032015 0300 25102015 0400 %  Bulgarien
5300000 +0200 +0300 29032015 0300 25102015 0400 %  Rumaenien
5700000 +0300 +0400 29032015 0400 25102015 0500 %  Aserbeidschan
5800000 +0300 +0400 29032015 0200 25102015 0300 %  Armenien
5900000 +0300                                   %  Kirgisistan
6000000 +0000 +0100 29032015 0100 25102015 0200 %  Irland
6600000 +0500                                   %  Tadschikistan
6700000 +0500                                   %  Turkmenistan
7000000 +0000 +0100 29032015 0100 25102015 0200 %  Großbritannien
7300000 +0200 +0300 29032015 0300 25102015 0400 %  Griechenland
7500000 +0200 +0300 29032015 0300 25102015 0400 %  Türkei
8000000 +0100 +0200 29032015 0200 25102015 0300 %  Deutschland (MEZ=GMT+1)
9400000 +0000 +0100 29032015 0100 25102015 0200 %  Portugal
9500000 +0200 +0300 27032015 0300 25102015 0200 %  Israel
9600000 +0330 +0430 22032015 0100 22092015 0000 %  Iran
9700000 +0200 +0300 27032015 0100 30102015 0000 %  Syrien
9900000 +0300                                   %  Irak
%
1100000 0000000
3200000 0000000
3400000 0000000
5400000 0000000
6100000 0000000
6800000 0000000
7100000 0000000
7400000 0000000
7600000 0000000
8100000 0000000
9800000 0000000
9999999 0000000)";

  static char const* const BASIC_DATA_TEST_DATA = R"(%
14.12.2014
12.12.2015
JF077 EVA_PRD~RIS Server~RIS OEV IMM~~J15~077_001 000000 END)";
};

TEST_F(loader_hrd_timezones_test, eva_numbers) {}

}  // loader
}  // motis
}  // hrd
