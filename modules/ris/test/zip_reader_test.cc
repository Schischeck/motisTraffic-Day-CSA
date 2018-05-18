#include "gtest/gtest.h"

#include "websocketpp/common/md5.hpp"

#include "motis/ris/zip_reader.h"

using namespace websocketpp::md5;

namespace motis {
namespace ris {

TEST(ris_zip_reader, read_zip) {
  zip_reader r{"modules/ris/test_resources/test.zip"};
  std::optional<std::string_view> file_content;

  // f9735b6d730a91f9a77af2ac17fcb833  test.200.raw
  ASSERT_TRUE(file_content = r.read());
  EXPECT_EQ(
      std::string("f9735b6d730a91f9a77af2ac17fcb833"),
      md5_hash_hex(reinterpret_cast<unsigned char const*>(file_content->data()),
                   file_content->size()));

  // d5715e91ac27550e94728d3704bcac52  test.5000.raw
  ASSERT_TRUE(file_content = r.read());
  EXPECT_EQ(
      std::string("d5715e91ac27550e94728d3704bcac52"),
      md5_hash_hex(reinterpret_cast<unsigned char const*>(file_content->data()),
                   file_content->size()));
}

}  // namespace ris
}  // namespace motis
