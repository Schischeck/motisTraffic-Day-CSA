#include "motis/loader/parser.h"

namespace motis {
namespace loader {
namespace gtfs {

struct gtfs_parser : public format_parser {
  bool applicable(boost::filesystem::path const& path) override;
  std::vector<std::string> missing_files(
      boost::filesystem::path const& path) const override;
  void parse(boost::filesystem::path const& root,
             flatbuffers::FlatBufferBuilder&) override;
};

}  // namespace gtfs
}  // namespace loader
}  // namespace motis
