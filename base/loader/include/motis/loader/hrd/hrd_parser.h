#include "motis/loader/parser.h"

#include "boost/filesystem.hpp"
#include "motis/loader/hrd/parse_config_inheritance.h"

#include "flatbuffers/flatbuffers.h"

namespace motis {
namespace loader {
namespace hrd {

struct hrd_parser : public format_parser {
  bool applicable(boost::filesystem::path const& path) override;
  bool applicable(boost::filesystem::path const& path, config const& c);

  std::vector<std::string> missing_files(
      boost::filesystem::path const& hrd_root) const override;

  std::vector<std::string> missing_files(
      boost::filesystem::path const& hrd_root, config const& c) const;

  void parse(boost::filesystem::path const& hrd_root,
             flatbuffers64::FlatBufferBuilder&) override;
  void parse(boost::filesystem::path const& hrd_root,
             flatbuffers64::FlatBufferBuilder&, config const& c);
};
}  // namespace hrd
}  // namespace loader
}  // namespace motis
