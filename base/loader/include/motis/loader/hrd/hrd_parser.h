#include "motis/loader/parser.h"

#include "boost/filesystem.hpp"

#include "flatbuffers/flatbuffers.h"

namespace motis {
namespace loader {
namespace hrd {

struct hrd_parser : public format_parser {
  bool applicable(boost::filesystem::path const& path) override;

  std::vector<std::string> missing_files(
      boost::filesystem::path const& path) const override;

  void parse(boost::filesystem::path const& path,
             flatbuffers::FlatBufferBuilder&) override;
};

}  // namespace hrd
}  // namespace loader
}  // namespace motis
