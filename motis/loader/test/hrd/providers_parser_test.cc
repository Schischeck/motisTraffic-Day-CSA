#include "gtest/gtest.h"

#include "motis/loader/util.h"
#include "motis/loader/parsers/hrd/providers_parser.h"

namespace motis {
namespace loader {
namespace hrd {

TEST(loader_hrd_providers, simple) {
  char const* file_content =
      "00001 K 'DPN' L 'ABR' V 'ABELLIO Rail Mitteldeutschland GmbH'\n"
      "00001 : AM____\n"
      "00002 K 'DPN' L 'ABR' V 'ABELLIO Rail NRW GmbH'\n"
      "00002 : AR____\n"
      "00003 K 'DPN' L 'ag ' V 'agilis'\n"
      "00003 : A9____ XY____\n";

  auto providers = parse_providers({"betrieb.101", file_content});

  EXPECT_EQ(4u, providers.size());

  auto const& first = providers[raw_to_int<uint64_t>("AM____")];
  EXPECT_EQ("DPN", first.short_name);
  EXPECT_EQ("ABR", first.long_name);
  EXPECT_EQ("ABELLIO Rail Mitteldeutschland GmbH", first.full_name);

  auto const& second = providers[raw_to_int<uint64_t>("AR____")];
  EXPECT_EQ("DPN", second.short_name);
  EXPECT_EQ("ABR", second.long_name);
  EXPECT_EQ("ABELLIO Rail NRW GmbH", second.full_name);

  auto const& third = providers[raw_to_int<uint64_t>("A9____")];
  EXPECT_EQ("DPN", third.short_name);
  EXPECT_EQ("ag ", third.long_name);
  EXPECT_EQ("agilis", third.full_name);

  auto const& fourth = providers[raw_to_int<uint64_t>("XY____")];
  EXPECT_EQ("DPN", fourth.short_name);
  EXPECT_EQ("ag ", fourth.long_name);
  EXPECT_EQ("agilis", fourth.full_name);
}

}  // loader
}  // motis
}  // hrd
