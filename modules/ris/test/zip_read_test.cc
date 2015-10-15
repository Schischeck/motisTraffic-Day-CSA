#include "gtest/gtest.h"

#include "websocketpp/common/md5.hpp"

#include "motis/ris/zip_reader.h"

using namespace websocketpp::md5;

namespace motis {
namespace ris {

TEST(ris_zip_reader, read_zip) {
  auto file_contents = read_zip_file("modules/ris/test_resources/test.zip");

  ASSERT_EQ(2, file_contents.size());

  // f9735b6d730a91f9a77af2ac17fcb833  test.200.raw
  EXPECT_EQ(
      std::string("f9735b6d730a91f9a77af2ac17fcb833"),
      md5_hash_hex(reinterpret_cast<unsigned char*>(file_contents[0].data()),
                   file_contents[0].size()));

  // d5715e91ac27550e94728d3704bcac52  test.5000.raw
  EXPECT_EQ(
      std::string("d5715e91ac27550e94728d3704bcac52"),
      md5_hash_hex(reinterpret_cast<unsigned char*>(file_contents[1].data()),
                   file_contents[1].size()));
}

}  // namespace ris
}  // namespace motis
