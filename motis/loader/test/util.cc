#include <cstring>
#include <cinttypes>

#include "catch/catch.hpp"

#include "motis/loader/util.h"

using namespace parser;

namespace motis {
namespace loader {
namespace hrd {

TEST_CASE("bitset_to_string_and_back") {
  std::string bit_string = "0101010100101010";
  std::bitset<16> before(bit_string);

  REQUIRE(string_to_bitset<16>(bitset_to_string<16>(before).c_str()) == before);
}

TEST_CASE("string_to_int") {
  REQUIRE(string_to_int<uint16_t>("ab") == 97 + (98 << 8));
}
}  // hrd
}  // loader
}  // motis
